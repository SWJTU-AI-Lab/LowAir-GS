/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../MainWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_MainWindow_t {
    uint offsetsAndSizes[26];
    char stringdata0[11];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[14];
    char stringdata5[14];
    char stringdata6[15];
    char stringdata7[12];
    char stringdata8[6];
    char stringdata9[17];
    char stringdata10[13];
    char stringdata11[4];
    char stringdata12[15];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MainWindow_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
        QT_MOC_LITERAL(0, 10),  // "MainWindow"
        QT_MOC_LITERAL(11, 13),  // "onLoadMission"
        QT_MOC_LITERAL(25, 0),  // ""
        QT_MOC_LITERAL(26, 13),  // "onStartReplay"
        QT_MOC_LITERAL(40, 13),  // "onPauseReplay"
        QT_MOC_LITERAL(54, 13),  // "onResetReplay"
        QT_MOC_LITERAL(68, 14),  // "onFrameUpdated"
        QT_MOC_LITERAL(83, 11),  // "ReplayFrame"
        QT_MOC_LITERAL(95, 5),  // "frame"
        QT_MOC_LITERAL(101, 16),  // "onReplayFinished"
        QT_MOC_LITERAL(118, 12),  // "onFpsUpdated"
        QT_MOC_LITERAL(131, 3),  // "fps"
        QT_MOC_LITERAL(135, 14)   // "onParticleTick"
    },
    "MainWindow",
    "onLoadMission",
    "",
    "onStartReplay",
    "onPauseReplay",
    "onResetReplay",
    "onFrameUpdated",
    "ReplayFrame",
    "frame",
    "onReplayFinished",
    "onFpsUpdated",
    "fps",
    "onParticleTick"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   62,    2, 0x08,    1 /* Private */,
       3,    0,   63,    2, 0x08,    2 /* Private */,
       4,    0,   64,    2, 0x08,    3 /* Private */,
       5,    0,   65,    2, 0x08,    4 /* Private */,
       6,    1,   66,    2, 0x08,    5 /* Private */,
       9,    0,   69,    2, 0x08,    7 /* Private */,
      10,    1,   70,    2, 0x08,    8 /* Private */,
      12,    0,   73,    2, 0x08,   10 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float,   11,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.offsetsAndSizes,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MainWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'onLoadMission'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStartReplay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPauseReplay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onResetReplay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFrameUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const ReplayFrame &, std::false_type>,
        // method 'onReplayFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFpsUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<float, std::false_type>,
        // method 'onParticleTick'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onLoadMission(); break;
        case 1: _t->onStartReplay(); break;
        case 2: _t->onPauseReplay(); break;
        case 3: _t->onResetReplay(); break;
        case 4: _t->onFrameUpdated((*reinterpret_cast< std::add_pointer_t<ReplayFrame>>(_a[1]))); break;
        case 5: _t->onReplayFinished(); break;
        case 6: _t->onFpsUpdated((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 7: _t->onParticleTick(); break;
        default: ;
        }
    }
}

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
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
