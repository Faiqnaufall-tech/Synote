/****************************************************************************
** Meta object code from reading C++ file 'AppWindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/AppWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AppWindow.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_AppWindow_t {
    uint offsetsAndSizes[30];
    char stringdata0[10];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[11];
    char stringdata4[15];
    char stringdata5[17];
    char stringdata6[5];
    char stringdata7[16];
    char stringdata8[5];
    char stringdata9[10];
    char stringdata10[15];
    char stringdata11[14];
    char stringdata12[10];
    char stringdata13[9];
    char stringdata14[13];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_AppWindow_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_AppWindow_t qt_meta_stringdata_AppWindow = {
    {
        QT_MOC_LITERAL(0, 9),  // "AppWindow"
        QT_MOC_LITERAL(10, 9),  // "onNewNote"
        QT_MOC_LITERAL(20, 0),  // ""
        QT_MOC_LITERAL(21, 10),  // "onSaveNote"
        QT_MOC_LITERAL(32, 14),  // "onNoteSelected"
        QT_MOC_LITERAL(47, 16),  // "QListWidgetItem*"
        QT_MOC_LITERAL(64, 4),  // "item"
        QT_MOC_LITERAL(69, 15),  // "onSearchChanged"
        QT_MOC_LITERAL(85, 4),  // "text"
        QT_MOC_LITERAL(90, 9),  // "onSyncNow"
        QT_MOC_LITERAL(100, 14),  // "onOpenSettings"
        QT_MOC_LITERAL(115, 13),  // "onAiSummarize"
        QT_MOC_LITERAL(129, 9),  // "onAiDraft"
        QT_MOC_LITERAL(139, 8),  // "onAiTodo"
        QT_MOC_LITERAL(148, 12)   // "onAddProject"
    },
    "AppWindow",
    "onNewNote",
    "",
    "onSaveNote",
    "onNoteSelected",
    "QListWidgetItem*",
    "item",
    "onSearchChanged",
    "text",
    "onSyncNow",
    "onOpenSettings",
    "onAiSummarize",
    "onAiDraft",
    "onAiTodo",
    "onAddProject"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_AppWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x08,    1 /* Private */,
       3,    0,   75,    2, 0x08,    2 /* Private */,
       4,    1,   76,    2, 0x08,    3 /* Private */,
       7,    1,   79,    2, 0x08,    5 /* Private */,
       9,    0,   82,    2, 0x08,    7 /* Private */,
      10,    0,   83,    2, 0x08,    8 /* Private */,
      11,    0,   84,    2, 0x08,    9 /* Private */,
      12,    0,   85,    2, 0x08,   10 /* Private */,
      13,    0,   86,    2, 0x08,   11 /* Private */,
      14,    0,   87,    2, 0x08,   12 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject AppWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_AppWindow.offsetsAndSizes,
    qt_meta_data_AppWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_AppWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AppWindow, std::true_type>,
        // method 'onNewNote'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSaveNote'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onNoteSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'onSearchChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onSyncNow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onOpenSettings'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAiSummarize'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAiDraft'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAiTodo'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAddProject'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void AppWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AppWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onNewNote(); break;
        case 1: _t->onSaveNote(); break;
        case 2: _t->onNoteSelected((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 3: _t->onSearchChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->onSyncNow(); break;
        case 5: _t->onOpenSettings(); break;
        case 6: _t->onAiSummarize(); break;
        case 7: _t->onAiDraft(); break;
        case 8: _t->onAiTodo(); break;
        case 9: _t->onAddProject(); break;
        default: ;
        }
    }
}

const QMetaObject *AppWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AppWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AppWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int AppWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
