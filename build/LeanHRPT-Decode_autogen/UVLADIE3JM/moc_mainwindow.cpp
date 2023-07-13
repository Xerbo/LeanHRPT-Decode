/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[39];
    char stringdata0[1045];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 23), // "on_actionOpen_triggered"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 37), // "on_actionSave_Current_Image_t..."
QT_MOC_LITERAL(4, 74, 36), // "on_actionSave_All_Channels_tr..."
QT_MOC_LITERAL(5, 111, 32), // "on_actionSave_GCP_File_triggered"
QT_MOC_LITERAL(6, 144, 28), // "on_actionProjector_triggered"
QT_MOC_LITERAL(7, 173, 32), // "on_actionMap_Shapefile_triggered"
QT_MOC_LITERAL(8, 206, 32), // "on_actionLandmark_File_triggered"
QT_MOC_LITERAL(9, 239, 28), // "on_actionMap_Color_triggered"
QT_MOC_LITERAL(10, 268, 33), // "on_actionLandmark_Color_trigg..."
QT_MOC_LITERAL(11, 302, 29), // "on_actionEnable_Map_triggered"
QT_MOC_LITERAL(12, 332, 35), // "on_actionEnable_Landmarks_tri..."
QT_MOC_LITERAL(13, 368, 23), // "on_actionFlip_triggered"
QT_MOC_LITERAL(14, 392, 26), // "on_actionCorrect_triggered"
QT_MOC_LITERAL(15, 419, 27), // "on_actionIR_Blend_triggered"
QT_MOC_LITERAL(16, 447, 26), // "on_groupProtocol_triggered"
QT_MOC_LITERAL(17, 474, 32), // "on_actionDocumentation_triggered"
QT_MOC_LITERAL(18, 507, 32), // "on_actionIssue_Tracker_triggered"
QT_MOC_LITERAL(19, 540, 33), // "on_actionAbout_LeanHRPT_trigg..."
QT_MOC_LITERAL(20, 574, 27), // "on_actionAbout_Qt_triggered"
QT_MOC_LITERAL(21, 602, 28), // "on_channelSelector_activated"
QT_MOC_LITERAL(22, 631, 5), // "index"
QT_MOC_LITERAL(23, 637, 24), // "on_redSelector_activated"
QT_MOC_LITERAL(24, 662, 26), // "on_greenSelector_activated"
QT_MOC_LITERAL(25, 689, 25), // "on_blueSelector_activated"
QT_MOC_LITERAL(26, 715, 27), // "on_equalisationNone_clicked"
QT_MOC_LITERAL(27, 743, 30), // "on_equalisationStretch_clicked"
QT_MOC_LITERAL(28, 774, 32), // "on_equalisationHistogram_clicked"
QT_MOC_LITERAL(29, 807, 25), // "on_zoomSelector_activated"
QT_MOC_LITERAL(30, 833, 27), // "on_imageTabs_currentChanged"
QT_MOC_LITERAL(31, 861, 31), // "on_presetSelector_textActivated"
QT_MOC_LITERAL(32, 893, 4), // "text"
QT_MOC_LITERAL(33, 898, 23), // "on_presetReload_clicked"
QT_MOC_LITERAL(34, 922, 25), // "on_gradient_textActivated"
QT_MOC_LITERAL(35, 948, 29), // "on_contrastLimit_valueChanged"
QT_MOC_LITERAL(36, 978, 5), // "value"
QT_MOC_LITERAL(37, 984, 29), // "on_contrastLimitApply_clicked"
QT_MOC_LITERAL(38, 1014, 30) // "on_brightnessOnly_stateChanged"

    },
    "MainWindow\0on_actionOpen_triggered\0\0"
    "on_actionSave_Current_Image_triggered\0"
    "on_actionSave_All_Channels_triggered\0"
    "on_actionSave_GCP_File_triggered\0"
    "on_actionProjector_triggered\0"
    "on_actionMap_Shapefile_triggered\0"
    "on_actionLandmark_File_triggered\0"
    "on_actionMap_Color_triggered\0"
    "on_actionLandmark_Color_triggered\0"
    "on_actionEnable_Map_triggered\0"
    "on_actionEnable_Landmarks_triggered\0"
    "on_actionFlip_triggered\0"
    "on_actionCorrect_triggered\0"
    "on_actionIR_Blend_triggered\0"
    "on_groupProtocol_triggered\0"
    "on_actionDocumentation_triggered\0"
    "on_actionIssue_Tracker_triggered\0"
    "on_actionAbout_LeanHRPT_triggered\0"
    "on_actionAbout_Qt_triggered\0"
    "on_channelSelector_activated\0index\0"
    "on_redSelector_activated\0"
    "on_greenSelector_activated\0"
    "on_blueSelector_activated\0"
    "on_equalisationNone_clicked\0"
    "on_equalisationStretch_clicked\0"
    "on_equalisationHistogram_clicked\0"
    "on_zoomSelector_activated\0"
    "on_imageTabs_currentChanged\0"
    "on_presetSelector_textActivated\0text\0"
    "on_presetReload_clicked\0"
    "on_gradient_textActivated\0"
    "on_contrastLimit_valueChanged\0value\0"
    "on_contrastLimitApply_clicked\0"
    "on_brightnessOnly_stateChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      34,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  184,    2, 0x08 /* Private */,
       3,    0,  185,    2, 0x08 /* Private */,
       4,    0,  186,    2, 0x08 /* Private */,
       5,    0,  187,    2, 0x08 /* Private */,
       6,    0,  188,    2, 0x08 /* Private */,
       7,    0,  189,    2, 0x08 /* Private */,
       8,    0,  190,    2, 0x08 /* Private */,
       9,    0,  191,    2, 0x08 /* Private */,
      10,    0,  192,    2, 0x08 /* Private */,
      11,    0,  193,    2, 0x08 /* Private */,
      12,    0,  194,    2, 0x08 /* Private */,
      13,    0,  195,    2, 0x08 /* Private */,
      14,    0,  196,    2, 0x08 /* Private */,
      15,    0,  197,    2, 0x08 /* Private */,
      16,    0,  198,    2, 0x08 /* Private */,
      17,    0,  199,    2, 0x08 /* Private */,
      18,    0,  200,    2, 0x08 /* Private */,
      19,    0,  201,    2, 0x08 /* Private */,
      20,    0,  202,    2, 0x08 /* Private */,
      21,    1,  203,    2, 0x08 /* Private */,
      23,    1,  206,    2, 0x08 /* Private */,
      24,    1,  209,    2, 0x08 /* Private */,
      25,    1,  212,    2, 0x08 /* Private */,
      26,    0,  215,    2, 0x08 /* Private */,
      27,    0,  216,    2, 0x08 /* Private */,
      28,    0,  217,    2, 0x08 /* Private */,
      29,    1,  218,    2, 0x08 /* Private */,
      30,    1,  221,    2, 0x08 /* Private */,
      31,    1,  224,    2, 0x08 /* Private */,
      33,    0,  227,    2, 0x08 /* Private */,
      34,    1,  228,    2, 0x08 /* Private */,
      35,    1,  231,    2, 0x08 /* Private */,
      37,    0,  234,    2, 0x08 /* Private */,
      38,    0,  235,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::QString,   32,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   32,
    QMetaType::Void, QMetaType::Int,   36,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->on_actionOpen_triggered(); break;
        case 1: _t->on_actionSave_Current_Image_triggered(); break;
        case 2: _t->on_actionSave_All_Channels_triggered(); break;
        case 3: _t->on_actionSave_GCP_File_triggered(); break;
        case 4: _t->on_actionProjector_triggered(); break;
        case 5: _t->on_actionMap_Shapefile_triggered(); break;
        case 6: _t->on_actionLandmark_File_triggered(); break;
        case 7: _t->on_actionMap_Color_triggered(); break;
        case 8: _t->on_actionLandmark_Color_triggered(); break;
        case 9: _t->on_actionEnable_Map_triggered(); break;
        case 10: _t->on_actionEnable_Landmarks_triggered(); break;
        case 11: _t->on_actionFlip_triggered(); break;
        case 12: _t->on_actionCorrect_triggered(); break;
        case 13: _t->on_actionIR_Blend_triggered(); break;
        case 14: _t->on_groupProtocol_triggered(); break;
        case 15: _t->on_actionDocumentation_triggered(); break;
        case 16: _t->on_actionIssue_Tracker_triggered(); break;
        case 17: _t->on_actionAbout_LeanHRPT_triggered(); break;
        case 18: _t->on_actionAbout_Qt_triggered(); break;
        case 19: _t->on_channelSelector_activated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: _t->on_redSelector_activated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: _t->on_greenSelector_activated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 22: _t->on_blueSelector_activated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->on_equalisationNone_clicked(); break;
        case 24: _t->on_equalisationStretch_clicked(); break;
        case 25: _t->on_equalisationHistogram_clicked(); break;
        case 26: _t->on_zoomSelector_activated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 27: _t->on_imageTabs_currentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 28: _t->on_presetSelector_textActivated((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 29: _t->on_presetReload_clicked(); break;
        case 30: _t->on_gradient_textActivated((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 31: _t->on_contrastLimit_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 32: _t->on_contrastLimitApply_clicked(); break;
        case 33: _t->on_brightnessOnly_stateChanged(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 34)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 34;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 34)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 34;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
