/*
 * SPDX-FileCopyrightText: 2018 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * The box blur implementation is based on AlphaBoxBlur from Firefox.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// own
#include "breezeboxshadowrenderer.h"

// Qt
#include <QPainter>
#include <QtMath>

namespace Breeze
{
static inline int calculateBlurRadius(qreal stdDev)
{
    // See https://www.w3.org/TR/SVG11/filters.html#feGaussianBlurElement
    const qreal gaussianScaleFactor = (3.0 * qSqrt(2.0 * M_PI) / 4.0) * 1.5;
    return qMax(2, qFloor(stdDev * gaussianScaleFactor + 0.5));
}

static inline qreal calculateBlurStdDev(int radius)
{
    // See https://www.w3.org/TR/css-backgrounds-3/#shadow-blur
    return radius * 0.5;
}

static inline QSize calculateBlurExtent(int radius)
{
    const int blurRadius = calculateBlurRadius(calculateBlurStdDev(radius));
    return QSize(blurRadius, blurRadius);
}

struct BoxLobes {
    int left; ///< how many pixels sample to the left
    int right; ///< how many pixels sample to the right
};

/**
 * Compute box filter parameters.
 *
 * @param radius The blur radius.
 * @returns Parameters for three box filters.
 **/
static QVector<BoxLobes> computeLobes(int radius)
{
    const int blurRadius = calculateBlurRadius(calculateBlurStdDev(radius));
    const int z = blurRadius / 3;

    int major;
    int minor;
    int final;

    switch (blurRadius % 3) {
    case 0:
        major = z;
        minor = z;
        final = z;
        break;

    case 1:
        major = z + 1;
        minor = z;
        final = z;
        break;

    case 2:
        major = z + 1;
        minor = z;
        final = z + 1;
        break;

    default:
        Q_UNREACHABLE();
    }

    Q_ASSERT(major + minor + final == blurRadius);

    return {{major, minor}, {minor, major}, {final, final}};
}

/**
 * Process a row with a box filter.
 *
 * @param src The start of the row.
 * @param dst The destination.
 * @param width The width of the row, in pixels.
 * @param horizontalStride The number of bytes from one alpha value to the
 *    next alpha value.
 * @param verticalStride The number of bytes from one row to the next row.
 * @param lobes Params of the box filter.
 * @param transposeInput Whether the input is transposed.
 * @param transposeOutput Whether the output should be transposed.
 **/
static inline void boxBlurRowAlpha(const uint8_t *src,
                                   uint8_t *dst,
                                   int width,
                                   int horizontalStride,
                                   int verticalStride,
                                   const BoxLobes &lobes,
                                   bool transposeInput,
                                   bool transposeOutput)
{
    const int inputStep = transposeInput ? verticalStride : horizontalStride;
    const int outputStep = transposeOutput ? verticalStride : horizontalStride;

    const int boxSize = lobes.left + 1 + lobes.right;
    const int reciprocal = (1 << 24) / boxSize;

    uint32_t alphaSum = (boxSize + 1) / 2;

    const uint8_t *left = src;
    const uint8_t *right = src;
    uint8_t *out = dst;

    const uint8_t firstValue = src[0];
    const uint8_t lastValue = src[(width - 1) * inputStep];

    alphaSum += firstValue * lobes.left;

    const uint8_t *initEnd = src + (boxSize - lobes.left) * inputStep;
    while (right < initEnd) {
        alphaSum += *right;
        right += inputStep;
    }

    const uint8_t *leftEnd = src + boxSize * inputStep;
    while (right < leftEnd) {
        *out = (alphaSum * reciprocal) >> 24;
        alphaSum += *right - firstValue;
        right += inputStep;
        out += outputStep;
    }

    const uint8_t *centerEnd = src + width * inputStep;
    while (right < centerEnd) {
        *out = (alphaSum * reciprocal) >> 24;
        alphaSum += *right - *left;
        left += inputStep;
        right += inputStep;
        out += outputStep;
    }

    const uint8_t *rightEnd = dst + width * outputStep;
    while (out < rightEnd) {
        *out = (alphaSum * reciprocal) >> 24;
        alphaSum += lastValue - *left;
        left += inputStep;
        out += outputStep;
    }
}

/**
 * Blur the alpha channel of a given image.
 *
 * @param image The input image.
 * @param radius The blur radius.
 * @param rect Specifies what part of the image to blur. If nothing is provided, then
 *    the whole alpha channel of the input image will be blurred.
 **/
static inline void boxBlurAlpha(QImage &image, int radius, const QRect &rect = {})
{
    if (radius < 2) {
        return;
    }

    const QVector<BoxLobes> lobes = computeLobes(radius);

    const QRect blurRect = rect.isNull() ? image.rect() : rect;

    const int alphaOffset = QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3;
    const int width = blurRect.width();
    const int height = blurRect.height();
    const int rowStride = image.bytesPerLine();
    const int pixelStride = image.depth() >> 3;

    const int bufferStride = qMax(width, height) * pixelStride;
    QScopedPointer<uint8_t, QScopedPointerArrayDeleter<uint8_t>> buf(new uint8_t[2 * bufferStride]);
    uint8_t *buf1 = buf.data();
    uint8_t *buf2 = buf1 + bufferStride;

    // Blur the image in horizontal direction.
    for (int i = 0; i < height; ++i) {
        uint8_t *row = image.scanLine(blurRect.y() + i) + blurRect.x() * pixelStride + alphaOffset;
        boxBlurRowAlpha(row, buf1, width, pixelStride, rowStride, lobes[0], false, false);
        boxBlurRowAlpha(buf1, buf2, width, pixelStride, rowStride, lobes[1], false, false);
        boxBlurRowAlpha(buf2, row, width, pixelStride, rowStride, lobes[2], false, false);
    }

    // Blur the image in vertical direction.
    for (int i = 0; i < width; ++i) {
        uint8_t *column = image.scanLine(blurRect.y()) + (blurRect.x() + i) * pixelStride + alphaOffset;
        boxBlurRowAlpha(column, buf1, height, pixelStride, rowStride, lobes[0], true, false);
        boxBlurRowAlpha(buf1, buf2, height, pixelStride, rowStride, lobes[1], false, false);
        boxBlurRowAlpha(buf2, column, height, pixelStride, rowStride, lobes[2], false, true);
    }
}

static inline void mirrorTopLeftQuadrant(QImage &image)
{
    const int width = image.width();
    const int height = image.height();

    const int centerX = qCeil(width * 0.5);
    const int centerY = qCeil(height * 0.5);

    const int alphaOffset = QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3;
    const int stride = image.depth() >> 3;

    for (int y = 0; y < centerY; ++y) {
        uint8_t *in = image.scanLine(y) + alphaOffset;
        uint8_t *out = in + (width - 1) * stride;

        for (int x = 0; x < centerX; ++x, in += stride, out -= stride) {
            *out = *in;
        }
    }

    for (int y = 0; y < centerY; ++y) {
        const uint8_t *in = image.scanLine(y) + alphaOffset;
        uint8_t *out = image.scanLine(width - y - 1) + alphaOffset;

        for (int x = 0; x < width; ++x, in += stride, out += stride) {
            *out = *in;
        }
    }
}

static void renderShadow(QPainter *painter, const QRectF &rect, qreal borderRadius, const QPointF &offset, double radius, const QColor &color)
{
    const qreal dpr = painter->device()->devicePixelRatioF();
    const QSize inflation = calculateBlurExtent(radius);
    const QSize pixelSize = ((rect.size() + 2 * inflation) * dpr).toSize();
    const QSizeF size = QSizeF(pixelSize) / dpr;

    QImage shadow(pixelSize, QImage::Format_ARGB32_Premultiplied);
    shadow.setDevicePixelRatio(dpr);
    shadow.fill(Qt::transparent);

    QRectF boxRect(QPoint(0, 0), rect.size());
    boxRect.moveCenter(QRectF(QPoint(0, 0), size).center());

    const qreal xRadius = 2.0 * borderRadius / boxRect.width();
    const qreal yRadius = 2.0 * borderRadius / boxRect.height();

    QPainter shadowPainter;
    shadowPainter.begin(&shadow);
    shadowPainter.setRenderHint(QPainter::Antialiasing);
    shadowPainter.setPen(Qt::NoPen);
    shadowPainter.setBrush(Qt::black);
    shadowPainter.drawRoundedRect(boxRect, xRadius, yRadius);
    shadowPainter.end();

    // Because the shadow texture is symmetrical, that's enough to blur
    // only the top-left quadrant and then mirror it.
    const QRect blurRect(0, 0, std::ceil(shadow.width() * 0.5), std::ceil(shadow.height() * 0.5));
    const int scaledRadius = std::round(radius * dpr);
    boxBlurAlpha(shadow, scaledRadius, blurRect);
    mirrorTopLeftQuadrant(shadow);

    // Give the shadow a tint of the desired color.
    shadowPainter.begin(&shadow);
    shadowPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    shadowPainter.fillRect(shadow.rect(), color);
    shadowPainter.end();

    // Actually, present the shadow.
    QRectF shadowRect = shadow.rect();
    shadowRect.setSize(shadowRect.size() / dpr);
    shadowRect.moveCenter(rect.center() + offset);
    painter->drawImage(shadowRect, shadow);
}

void BoxShadowRenderer::setBoxSize(const QSizeF &size)
{
    m_boxSize = size;
}

void BoxShadowRenderer::setBorderRadius(qreal radius)
{
    m_borderRadius = radius;
}

void BoxShadowRenderer::addShadow(const QPointF &offset, double radius, const QColor &color)
{
    Shadow shadow = {};
    shadow.offset = offset;
    shadow.radius = radius;
    shadow.color = color;
    m_shadows.append(shadow);
}

QImage BoxShadowRenderer::render() const
{
    if (m_shadows.isEmpty()) {
        return {};
    }

    QSizeF canvasSize;
    for (const Shadow &shadow : std::as_const(m_shadows)) {
        canvasSize = canvasSize.expandedTo(calculateMinimumShadowTextureSize(m_boxSize, shadow.radius, shadow.offset));
    }

    QImage canvas(canvasSize.toSize(), QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);

    QRectF boxRect(QPoint(0, 0), m_boxSize);
    boxRect.moveCenter(QRect(QPoint(0, 0), canvas.size()).center());

    QPainter painter(&canvas);
    for (const Shadow &shadow : std::as_const(m_shadows)) {
        renderShadow(&painter, boxRect, m_borderRadius, shadow.offset, shadow.radius, shadow.color);
    }
    painter.end();

    return canvas;
}

QSize BoxShadowRenderer::calculateMinimumBoxSize(int radius)
{
    const QSize blurExtent = calculateBlurExtent(radius);
    return 2 * blurExtent + QSize(1, 1);
}

QSizeF BoxShadowRenderer::calculateMinimumShadowTextureSize(const QSizeF &boxSize, double radius, const QPointF &offset)
{
    return boxSize + 2 * calculateBlurExtent(radius) + QSizeF(std::abs(offset.x()), std::abs(offset.y()));
}

} // namespace Breeze
