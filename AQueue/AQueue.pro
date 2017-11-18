TARGET = AsynQueue
TEMPLATE = lib
CONFIG += plugin
DESTDIR = ../../../lib
OBJECTS_DIR = ../../../tmp/Debug/AsynQueue
win32:LIBS += -lwsock32 \
    -lws2_32 \
    -lAdvapi32
INCLUDEPATH += ./inc
HEADERS += inc/yqueue.hpp \
    inc/ypipe.hpp \
    inc/windows.hpp \
    inc/thread.hpp \
    inc/stdint.hpp \
    inc/signaler.hpp \
    inc/select.hpp \
    inc/poller_base.hpp \
    inc/poller.hpp \
    inc/platform.hpp \
    inc/mutex.hpp \
    inc/mailbox.hpp \
    inc/likely.hpp \
    inc/i_poll_events.hpp \
    inc/ip.hpp \
    inc/io_thread.hpp \
    inc/fd.hpp \
    inc/err.hpp \
    inc/ctx.hpp \
    inc/config.hpp \
    inc/command.hpp \
    inc/clock.hpp \
    inc/atomic_ptr.hpp \
    inc/atomic_counter.hpp \
    inc/epoll.hpp \
    inc/aqueue.h \
    inc/aqueue_msg.h \
    inc/i_aqueue_callback.h \
    inc/a_object.hpp
SOURCES += src/thread.cpp \
    src/signaler.cpp \
    src/select.cpp \
    src/poller_base.cpp \
    src/mailbox.cpp \
    src/ip.cpp \
    src/io_thread.cpp \
    src/err.cpp \
    src/ctx.cpp \
    src/clock.cpp \
    src/amq.cpp \
    src/epoll.cpp \
    src/a_object.cpp
