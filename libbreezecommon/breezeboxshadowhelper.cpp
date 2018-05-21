/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "breezeboxshadowhelper.h"
#include "config-breezecommon.h"

#include <QVector>

#include <fftw3.h>

#include <cmath>


namespace Breeze {
namespace BoxShadowHelper {

namespace {
    // FFT approach outperforms naive blur method when blur radius >= 64.
    // (was discovered after doing a lot of benchmarks)
    const int FFT_BLUR_RADIUS_THRESHOLD = 64;

    // According to the CSS Level 3 spec, standard deviation must be equal to
    // half of the blur radius. https://www.w3.org/TR/css-backgrounds-3/#shadow-blur
    // Current window size is too small for sigma equal to half of the blur radius.
    // As a workaround, sigma blur scale is lowered. With the lowered sigma
    // blur scale, area under the kernel equals to 0.98, which is pretty enough.
    // Maybe, it should be changed in the future.
    const double SIGMA_BLUR_SCALE = 0.4375;
}

inline int kernelSizeToRadius(int kernelSize)
{
    return (kernelSize - 1) / 2;
}

inline int radiusToKernelSize(int radius)
{
    return radius * 2 + 1;
}

QVector<double> computeGaussianKernel(int radius)
{
    QVector<double> kernel;
    const int kernelSize = radiusToKernelSize(radius);
    kernel.reserve(kernelSize);

    const double sigma = SIGMA_BLUR_SCALE * radius;
    const double den = std::sqrt(2.0) * sigma;
    double kernelNorm = 0.0;
    double lastInt = 0.5 * std::erf((-radius - 0.5) / den);

    for (int i = 0; i < kernelSize; i++) {
        const double currInt = 0.5 * std::erf((i - radius + 0.5) / den);
        const double w = currInt - lastInt;
        kernel << w;
        kernelNorm += w;
        lastInt = currInt;
    }

    for (auto &w : kernel) {
        w /= kernelNorm;
    }

    return kernel;
}

// Do horizontal pass of the Gaussian filter. Please notice that the result
// is transposed. So, the dst image should have proper size, e.g. if the src
// image have (wxh) size then the dst image should have (hxw) size. The
// result is transposed so we read memory in linear order.
void blurAlphaNaivePass(const QImage &src, QImage &dst, const QVector<qreal> &kernel)
{
    const int alphaOffset = QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3;
    const int alphaStride = src.depth() >> 3;
    const int radius = kernelSizeToRadius(kernel.size());

    for (int y = 0; y < src.height(); y++) {
        const uchar *in = src.scanLine(y) + alphaOffset;
        uchar *out = dst.scanLine(0) + alphaOffset + y * alphaStride;

        for (int x = 0; x < radius; x++) {
            const uchar *window = in;
            qreal alpha = 0;
            for (int k = radius - x; k < kernel.size(); k++) {
                alpha += *window * kernel[k];
                window += alphaStride;
            }
            *out = static_cast<uchar>(alpha);
            out += dst.width() * alphaStride;
        }

        for (int x = radius; x < src.width() - radius; x++) {
            const uchar *window = in + (x - radius) * alphaStride;
            qreal alpha = 0;
            for (int k = 0; k < kernel.size(); k++) {
                alpha += *window * kernel[k];
                window += alphaStride;
            }
            *out = static_cast<uchar>(alpha);
            out += dst.width() * alphaStride;
        }

        for (int x = src.width() - radius; x < src.width(); x++) {
            const uchar *window = in + (x - radius - 1) * alphaStride;
            qreal alpha = 0;
            const int outside = x + radius - src.width();
            for (int k = 0; k < kernel.size() - outside; k++) {
                alpha += *window * kernel[k];
                window += alphaStride;
            }
            *out = static_cast<uchar>(alpha);
            out += dst.width() * alphaStride;
        }
    }
}

// Blur alpha channel of the given image using separable convolution
// gaussian kernel. Not very efficient with big blur radii.
void blurAlphaNaive(QImage &img, int radius)
{
    const QVector<qreal> kernel = computeGaussianKernel(radius);
    QImage tmp(img.height(), img.width(), img.format());

    blurAlphaNaivePass(img, tmp, kernel); // horizontal pass
    blurAlphaNaivePass(tmp, img, kernel); // vertical pass
}

// Blur alpha channel of the given image using Fourier Transform.
// It's somewhat efficient with big blur radii.
//
// It works as follows:
//   - do FFT on given input image(it is expected, that the
//     input image was padded before)
//   - compute Gaussian kernel, pad it to the size of the input
//     image, and do FFT on it
//   - multiply the two in the frequency domain(element-wise)
//   - transform the result back to "time domain"
//
void blurAlphaFFT(QImage &img, int radius)
{
    const int alphaOffset = QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3;
    const int alphaStride = img.depth() >> 3;
    const int size = img.width() * img.height();

    // Use FFTW's malloc function so the returned pointer obeys any
    // special alignment restrictions. (e.g. for SIMD acceleration, etc)
    // See http://www.fftw.org/fftw3_doc/MekernelSizeToRadius(mory-Allocation.html
    fftw_complex *imageIn = fftw_alloc_complex(size);
    fftw_complex *imageOut = fftw_alloc_complex(size);

    uchar *data = img.scanLine(0) + alphaOffset;
    for (int i = 0; i < size; i++) {
        imageIn[i][0] = *data;
        imageIn[i][1] = 0.0;
        data += alphaStride;
    }

    fftw_plan imageFFT = fftw_plan_dft_2d(
        img.height(), img.width(),
        imageIn, imageOut,
        FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_plan imageIFFT = fftw_plan_dft_2d(
        img.height(), img.width(),
        imageOut, imageIn,
        FFTW_BACKWARD, FFTW_ESTIMATE);

    // The computed Gaussian kernel has to have the same size as the input image.
    // Please note that the center of the computed Gaussian kernel is placed
    // at the top-left corner and the whole kernel is wrapped around so we read
    // result in linear order.
    // Note: the kernel is computed by taking a product of two 1-D Gaussian kernels.
    QVector<double> kernel(size, 0);
    const QVector<double> kernel_ = computeGaussianKernel(radius);
    for (int y = 0; y < kernel_.size(); y++) {
        const int i = (img.height() + y - radius) % img.height();
        for (int x = 0; x < kernel_.size(); x++) {
            const int j = (img.width() + x - radius) % img.width();
            kernel[j + i * img.width()] = kernel_[x] * kernel_[y];
        }
    }

    fftw_complex *kernelIn = fftw_alloc_complex(kernel.size());
    fftw_complex *kernelOut = fftw_alloc_complex(kernel.size());

    for (int i = 0; i < size; i++) {
        kernelIn[i][0] = kernel[i];
        kernelIn[i][1] = 0.0;
    }

    fftw_plan kernelFFT = fftw_plan_dft_2d(
        img.height(), img.width(),
        kernelIn, kernelOut,
        FFTW_FORWARD, FFTW_ESTIMATE);

    // Do actual FFT.
    fftw_execute(imageFFT);
    fftw_execute(kernelFFT);

    for (int i = 0; i < size; i++) {
        const double re = imageOut[i][0] * kernelOut[i][0] - imageOut[i][1] * kernelOut[i][1];
        const double im = imageOut[i][0] * kernelOut[i][1] + imageOut[i][1] * kernelOut[i][0];
        imageOut[i][0] = re;
        imageOut[i][1] = im;
    }

    fftw_execute(imageIFFT);

    // Copy result back. Please note, result is scaled by `width x height` so we need to scale it down.
    const qreal invSize = 1.0 / size;
    data = img.scanLine(0) + alphaOffset;
    for (int i = 0; i < size; i++) {
        *data = imageIn[i][0] * invSize;
        data += alphaStride;
    }

    fftw_destroy_plan(kernelFFT);
    fftw_destroy_plan(imageFFT);
    fftw_destroy_plan(imageIFFT);

    fftw_free(kernelIn);
    fftw_free(kernelOut);

    fftw_free(imageIn);
    fftw_free(imageOut);
}

void boxShadow(QPainter *p, const QRect &box, const QPoint &offset, int radius, const QColor &color)
{
    const QSize size = box.size() + 2 * QSize(radius, radius);

#if BREEZE_COMMON_USE_KDE4
    const qreal dpr = 1.0;
#else
    const qreal dpr = p->device()->devicePixelRatioF();
#endif

    QPainter painter;

    QImage shadow(size * dpr, QImage::Format_ARGB32_Premultiplied);
#if !BREEZE_COMMON_USE_KDE4
    shadow.setDevicePixelRatio(dpr);
#endif
    shadow.fill(Qt::transparent);

    painter.begin(&shadow);
    painter.fillRect(QRect(QPoint(radius, radius), box.size()), Qt::black);
    painter.end();

    // There is no need to blur RGB channels. Blur the alpha
    // channel and then give the shadow a tint of the desired color.
    const int radius_ = radius * dpr;
    if (radius_ < FFT_BLUR_RADIUS_THRESHOLD) {
        blurAlphaNaive(shadow, radius_);
    } else {
        blurAlphaFFT(shadow, radius_);
    }

    painter.begin(&shadow);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(shadow.rect(), color);
    painter.end();

    QRect shadowRect = shadow.rect();
    shadowRect.setSize(shadowRect.size() / dpr);
    shadowRect.moveCenter(box.center() + offset);
    p->drawImage(shadowRect, shadow);
}

} // BoxShadowHelper
} // Breeze
