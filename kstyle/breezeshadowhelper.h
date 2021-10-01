/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org> 
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breezeshadowhelper_h
#define breezeshadowhelper_h

#include "breezetileset.h"

#include <KWindowShadow>

#include <QObject>
#include <QPointer>
#include <QMap>
#include <QMargins>
#include <QSet>

namespace Breeze
{

    //* forward declaration
    class Helper;

    struct ShadowParams
    {
        ShadowParams() = default;

        ShadowParams(const QPoint &offset, int radius, qreal opacity):
            offset(offset),
            radius(radius),
            opacity(opacity)
        {}

        QPoint offset;
        int radius = 0;
        qreal opacity = 0;

        void operator*=(qreal factor) {
            offset *= factor;
            radius = qRound(radius * factor);
        }
    };

    struct CompositeShadowParams
    {
        CompositeShadowParams() = default;

        CompositeShadowParams(
                const QPoint &offset,
                const ShadowParams &shadow1,
                const ShadowParams &shadow2)
            : offset(offset)
            , shadow1(shadow1)
            , shadow2(shadow2) {}

        bool isNone() const
        { return qMax(shadow1.radius, shadow2.radius) == 0; }

        QPoint offset;
        ShadowParams shadow1;
        ShadowParams shadow2;

        void operator*=(qreal factor) {
            offset *= factor;
            shadow1 *= factor;
            shadow2 *= factor;
        }
    };

    //* handle shadow pixmaps passed to window manager via X property
    class ShadowHelper: public QObject
    {

        Q_OBJECT

        public:

        //* constructor
        ShadowHelper( QObject*, Helper& );

        //* destructor
        ~ShadowHelper() override;

        //* shadow params from size enum
        static CompositeShadowParams lookupShadowParams( int shadowSizeEnum );

        //* reset
        void reset();

        //* load config
        void loadConfig();

        //* register widget
        bool registerWidget( QWidget*, bool force = false );

        //* unregister widget
        void unregisterWidget( QWidget* );

        //* event filter
        bool eventFilter( QObject*, QEvent* ) override;

        //* shadow tiles
        /** is public because it is also needed for mdi windows */
        TileSet shadowTiles(QWidget *);

        //* return device pixel ratio for the window containing the widget
        static qreal devicePixelRatio(QWidget *) ;

        protected Q_SLOTS:

        //* unregister widget
        void widgetDeleted( QObject* );

        //* unregister window
        void windowDeleted( QObject* );

        protected:

        //* true if widget is a menu
        bool isMenu( QWidget* ) const;

        //* true if widget is a tooltip
        bool isToolTip( QWidget* ) const;

        //* dock widget
        bool isDockWidget( QWidget* ) const;

        //* toolbar
        bool isToolBar( QWidget* ) const;

        //* accept widget
        bool acceptWidget( QWidget* ) const;

        // create shared shadow tiles from tileset
        const QVector<KWindowShadowTile::Ptr>& createShadowTiles();

        // create shadow tile from pixmap
        KWindowShadowTile::Ptr createTile( const QPixmap& );

        //* installs shadow on given widget in a platform independent way
        void installShadows( QWidget * );

        //* uninstalls shadow on given widget in a platform independent way
        void uninstallShadows( QWidget * );

        //* gets the shadow margins for the given widget
        QMargins shadowMargins( QWidget* ) const;

        private:

        //* helper
        Helper& _helper;

        //* registered widgets
        QSet<QWidget*> _widgets;

        //* managed shadows
        QMap<QWindow*, KWindowShadow*> _shadows;

        //* tileset
        TileSet _shadowTiles;

        //* number of tiles
        enum { numTiles = 8 };

        //* shared shadow tiles
        QVector<KWindowShadowTile::Ptr> _tiles;

    };

}

#endif
