/****************************************************************************
**
** Copyright (C) 2014-2018 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of Live CV Application.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#include "lcvcore_plugin.h"
#include "qmat.h"
#include "qwritablemat.h"
#include "qmatop.h"
#include "qmatext.h"
#include "qmatview.h"
#include "qimread.h"
#include "qimwrite.h"
#include "qmatroi.h"
#include "qmatread.h"
#include "qcamcapture.h"
#include "qvideocapture.h"
#include "qvideowriter.h"
#include "qmatbuffer.h"
#include "qalphamerge.h"
#include "qabsdiff.h"
#include "qdrawhistogram.h"
#include "qcolorhistogram.h"
#include "qmatloader.h"
#include "qmatlist.h"
#include "qimagefile.h"
#include "qoverlapmat.h"
#include "qitemcapture.h"

#include "live/viewengine.h"
#include "live/viewcontext.h"

#include <qqml.h>
#include <QQmlEngine>


static QObject* matOpProvider(QQmlEngine *engine, QJSEngine *){
    return new QMatOp(engine);
}

void LcvcorePlugin::registerTypes(const char *uri){
    // @uri modules.lcvcore
    qmlRegisterType<QMat>(                   uri, 1, 0, "Mat");
    qmlRegisterType<QWritableMat>(           uri, 1, 0, "WritableMat");
    qmlRegisterType<QMatView>(               uri, 1, 0, "MatView");
    qmlRegisterType<QImRead>(                uri, 1, 0, "ImRead");
    qmlRegisterType<QImWrite>(               uri, 1, 0, "ImWrite");
    qmlRegisterType<QMatRoi>(                uri, 1, 0, "MatRoi");
    qmlRegisterType<QMatRead>(               uri, 1, 0, "MatRead");
    qmlRegisterType<QCamCapture>(            uri, 1, 0, "CamCapture");
    qmlRegisterType<QVideoCapture>(          uri, 1, 0, "VideoCapture");
    qmlRegisterType<QVideoWriter>(           uri, 1, 0, "VideoWriter");
    qmlRegisterType<QMatBuffer>(             uri, 1, 0, "MatBuffer");
    qmlRegisterType<QAlphaMerge>(            uri, 1, 0, "AlphaMerge");
    qmlRegisterType<QAbsDiff>(               uri, 1, 0, "AbsDiff");
    qmlRegisterType<QDrawHistogram>(         uri, 1, 0, "DrawHistogram");
    qmlRegisterType<QColorHistogram>(        uri, 1, 0, "ColorHistogram");
    qmlRegisterType<QMatList>(               uri, 1, 0, "MatList");
    qmlRegisterType<QMatLoader>(             uri, 1, 0, "MatLoader");
    qmlRegisterType<QImageFile>(             uri, 1, 0, "ImageFile");
    qmlRegisterType<QOverlapMat>(            uri, 1, 0, "OverlapMat");
    qmlRegisterType<QItemCapture>(           uri, 1, 0, "ItemCapture");

    qmlRegisterSingletonType<QMatOp>(        uri, 1, 0, "MatOp", &matOpProvider);
}

void LcvcorePlugin::initializeEngine(QQmlEngine *, const char *){
    lv::ViewContext::instance().engine()->registerQmlTypeInfo<QMat>(
        &lv::ml::serialize<QMat>,
        &lv::ml::deserialize<QMat>,
        [](){return new QMat;},
        true
    );
}


