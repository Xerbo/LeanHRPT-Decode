/****************************************************************************
** Meta object code from reading C++ file 'projectdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/projectdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'projectdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ProjectDialog_t {
    QByteArrayData data[18];
    char stringdata0[252];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ProjectDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ProjectDialog_t qt_meta_stringdata_ProjectDialog = {
    {
QT_MOC_LITERAL(0, 0, 13), // "ProjectDialog"
QT_MOC_LITERAL(1, 14, 12), // "get_viewport"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 10), // "get_points"
QT_MOC_LITERAL(4, 39, 36), // "std::vector<std::pair<xy,Geod..."
QT_MOC_LITERAL(5, 76, 6), // "size_t"
QT_MOC_LITERAL(6, 83, 1), // "n"
QT_MOC_LITERAL(7, 85, 13), // "map_shapefile"
QT_MOC_LITERAL(8, 99, 9), // "map_color"
QT_MOC_LITERAL(9, 109, 10), // "map_enable"
QT_MOC_LITERAL(10, 120, 13), // "landmark_file"
QT_MOC_LITERAL(11, 134, 14), // "landmark_color"
QT_MOC_LITERAL(12, 149, 15), // "landmark_enable"
QT_MOC_LITERAL(13, 165, 16), // "default_filename"
QT_MOC_LITERAL(14, 182, 18), // "on_preview_clicked"
QT_MOC_LITERAL(15, 201, 17), // "on_render_clicked"
QT_MOC_LITERAL(16, 219, 27), // "on_projection_textActivated"
QT_MOC_LITERAL(17, 247, 4) // "text"

    },
    "ProjectDialog\0get_viewport\0\0get_points\0"
    "std::vector<std::pair<xy,Geodetic> >\0"
    "size_t\0n\0map_shapefile\0map_color\0"
    "map_enable\0landmark_file\0landmark_color\0"
    "landmark_enable\0default_filename\0"
    "on_preview_clicked\0on_render_clicked\0"
    "on_projection_textActivated\0text"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ProjectDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x06 /* Public */,
       3,    1,   75,    2, 0x06 /* Public */,
       7,    0,   78,    2, 0x06 /* Public */,
       8,    0,   79,    2, 0x06 /* Public */,
       9,    0,   80,    2, 0x06 /* Public */,
      10,    0,   81,    2, 0x06 /* Public */,
      11,    0,   82,    2, 0x06 /* Public */,
      12,    0,   83,    2, 0x06 /* Public */,
      13,    0,   84,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      14,    0,   85,    2, 0x08 /* Private */,
      15,    0,   86,    2, 0x08 /* Private */,
      16,    1,   87,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::QImage,
    0x80000000 | 4, 0x80000000 | 5,    6,
    QMetaType::QString,
    QMetaType::QColor,
    QMetaType::Bool,
    QMetaType::QString,
    QMetaType::QColor,
    QMetaType::Bool,
    QMetaType::QString,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   17,

       0        // eod
};

void ProjectDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ProjectDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: { QImage _r = _t->get_viewport();
            if (_a[0]) *reinterpret_cast< QImage*>(_a[0]) = std::move(_r); }  break;
        case 1: { std::vector<std::pair<xy,Geodetic> > _r = _t->get_points((*reinterpret_cast< size_t(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< std::vector<std::pair<xy,Geodetic> >*>(_a[0]) = std::move(_r); }  break;
        case 2: { QString _r = _t->map_shapefile();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 3: { QColor _r = _t->map_color();
            if (_a[0]) *reinterpret_cast< QColor*>(_a[0]) = std::move(_r); }  break;
        case 4: { bool _r = _t->map_enable();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 5: { QString _r = _t->landmark_file();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 6: { QColor _r = _t->landmark_color();
            if (_a[0]) *reinterpret_cast< QColor*>(_a[0]) = std::move(_r); }  break;
        case 7: { bool _r = _t->landmark_enable();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 8: { QString _r = _t->default_filename();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 9: _t->on_preview_clicked(); break;
        case 10: _t->on_render_clicked(); break;
        case 11: _t->on_projection_textActivated((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = QImage (ProjectDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProjectDialog::get_viewport)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = std::vector<std::pair<xy,Geodetic>> (ProjectDialog::*)(size_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProjectDialog::get_points)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = QString (ProjectDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProjectDialog::map_shapefile)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = QColor (ProjectDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProjectDialog::map_color)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = bool (ProjectDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProjectDialog::map_enable)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = QString (ProjectDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProjectDialog::landmark_file)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = QColor (ProjectDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProjectDialog::landmark_color)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = bool (ProjectDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProjectDialog::landmark_enable)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = QString (ProjectDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ProjectDialog::default_filename)) {
                *result = 8;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ProjectDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ProjectDialog.data,
    qt_meta_data_ProjectDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ProjectDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProjectDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ProjectDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ProjectDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
QImage ProjectDialog::get_viewport()
{
    QImage _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
    return _t0;
}

// SIGNAL 1
std::vector<std::pair<xy,Geodetic>> ProjectDialog::get_points(size_t _t1)
{
    std::vector<std::pair<xy,Geodetic> > _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
    return _t0;
}

// SIGNAL 2
QString ProjectDialog::map_shapefile()
{
    QString _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
    return _t0;
}

// SIGNAL 3
QColor ProjectDialog::map_color()
{
    QColor _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
    return _t0;
}

// SIGNAL 4
bool ProjectDialog::map_enable()
{
    bool _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
    return _t0;
}

// SIGNAL 5
QString ProjectDialog::landmark_file()
{
    QString _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
    return _t0;
}

// SIGNAL 6
QColor ProjectDialog::landmark_color()
{
    QColor _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
    return _t0;
}

// SIGNAL 7
bool ProjectDialog::landmark_enable()
{
    bool _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
    return _t0;
}

// SIGNAL 8
QString ProjectDialog::default_filename()
{
    QString _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
    return _t0;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
