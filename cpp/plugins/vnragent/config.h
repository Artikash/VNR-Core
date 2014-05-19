#pragma once

// config.h
// 4/24/2014 jichi

/**
 *  IP and address of the VNR socket server.
 *  Must be consistent with the metacall port defined in reader.yaml
 */
#define VNRAGENT_SOCKET_PORT  6103
#define VNRAGENT_SOCKET_HOST  "127.0.0.1"

/**
 * Pipe name of the VNR socket server.
 */
// http://www.qtcentre.org/threads/14753-Named-pipes-in-Qt
//#define VNRAGENT_SOCKET_PIPE  "\\\\.\\pipe\\vnr.socket"
#define VNRAGENT_SOCKET_PIPE  "vnr.socket"

/**
 *  Shared memory key and max size.
 */
#define VNRAGENT_MEMORY_KEY "vnragent.%1" // vnragent.pid
#define VNRAGENT_MEMORY_SIZE 4096

/**
 *  Log qDebug message and save to a file.
 */
//#define VNRAGENT_DEBUG          // Log debug message
#define VNRAGENT_DEBUG_FILE     "vnragent.log"

/**
 *  Use TCP socket to communicate with VNR.
 *  When not enabled, use local pipe stead.
 */
//#define VNRAGENT_ENABLE_TCP_SOCKET

/**
 *  Put QCoreApplication on another thread
 *  This is not needed to translate UI text.
 *  But it is indispensible to hijack engine which require blocking the main thread.
 */
#define VNRAGENT_ENABLE_THREAD

/**
 *  Automatically reconnect on error.
 */
#define VNRAGENT_ENABLE_RECONNECT


/**
 *  Aware of the NTLEA bugs when the game is launched by ntleac.
 */
#define VNRAGENT_ENABLE_NTLEA

/**
 *  Unload the DLLs
 */
//#define VNRAGENT_ENABLE_UNLOAD

/**
 *  Use timer to drive event loop instead of exec.
 *  This is needed if VNRAGENT_ENABLE_THREAD is disabled.
 */
//#define VNRAGENT_ENABLE_APPRUNNER

#ifdef VNRAGENT_ENABLE_APPRUNNER
# define QT_EVENTLOOP_INTERVAL   10 // in ms
#endif // VNRAGENT_ENABLE_APPRUNNER

/**
 *  Allow sending time-critical message using native Windows pipe bypassing Qt
 *  Warning, communicate through native pipe will be fast but could contend with Qt messages
 *  as they are using different POVERLAPPED.
 *
 *  Enabling this will break CTRL detection.
 */
//#define VNRAGENT_ENABLE_NATIVE_PIPE

// EOF
