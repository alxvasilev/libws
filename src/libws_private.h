
#ifndef __LIBWS_PRIVATE_H__
#define __LIBWS_PRIVATE_H__

///
/// @internal
/// @file libws_private.h
///
/// @author Joakim Söderberg <joakim.soderberg@gmail.com>
///
///

#include "libws_config.h"
#include "libws_log.h"
#include "libws_types.h"
#include "libws_header.h"
#include "libws_utf8.h"
#include "libws_handshake.h"

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <event2/event.h>
#include <event2/bufferevent.h>

#ifdef LIBWS_WITH_OPENSSL
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <event2/bufferevent_ssl.h>
#endif // LIBWS_WITH_OPENSSL

#ifdef LIBWS_EXTERNAL_LOOP
//The option to add to bufferevent creation to enable thread safety of bufferevent
    #define _LIBWS_LE2_OPT_THREADSAFE BEV_OPT_THREADSAFE
#else
    #define _LIBWS_LE2_OPT_THREADSAFE 0
#endif

typedef enum ws_send_state_e
{
    WS_SEND_STATE_NONE,
    WS_SEND_STATE_MESSAGE_BEGIN,
    WS_SEND_STATE_IN_MESSAGE,
    WS_SEND_STATE_IN_MESSAGE_PAYLOAD
} ws_send_state_t;

typedef enum ws_connect_state_e
{
    WS_CONNECT_STATE_ERROR = -1,
    WS_CONNECT_STATE_NONE = 0,
    WS_CONNECT_STATE_SENT_REQ,
    WS_CONNECT_STATE_PARSED_STATUS,
    WS_CONNECT_STATE_PARSED_HEADERS,
    WS_CONNECT_STATE_HANDSHAKE_COMPLETE
} ws_connect_state_t;

///
/// Context for a websocket connection.
///
typedef struct ws_s
{
    struct ws_base_s *ws_base; ///< Base context that this
                               /// websocket session belongs to.

    ///
    /// @defgroup StateVariables State Variables
    /// @{
    ///
    ws_state_t state;                 ///< Websocket state.
    ws_connect_state_t connect_state; ///< Connection handshake state.
    void *user_state;
    /// @}
 
    ///
    /// @defgroup LibeventVariables Libevent Variables
    /// @{
    ///
    struct bufferevent *bev;    ///< Buffer event socket.
    /// @}

    ///
    /// @defgroup Callbacks Callback functions
    /// @{
    ///
    ws_msg_callback_f msg_cb;   ///< Callback for when a message
                                /// is received on the websocket.
    void *msg_arg;              ///< The user supplied argument to pass to the
                                /// the ws_s#msg_cb callback.
    ws_msg_begin_callback_f msg_begin_cb; ///< Message begin callback.
    void *msg_begin_arg;        ///< User supplied argument for
                                /// the ws_s#msg_begin_cb.
    ws_msg_frame_callback_f msg_frame_cb;
                                ///< Callback for when a frame in a message 
                                /// has arrived.
    void *msg_frame_arg;        ///< User supplied argument for
                                /// the ws_s#msg_frame_cb.
    ws_msg_end_callback_f msg_end_cb; ///< Callback for when a message ends.
    void *msg_end_arg;          ///< User supplied argument for
                                /// the ws_s#msg_end_cb.
    ws_msg_frame_begin_callback_f msg_frame_begin_cb; 
                                ///< Callback for when a frame begins (when the
                                /// header has been read).
    void *msg_frame_begin_arg;  ///< User supplied argument
                                /// for the ws_s#msg_frame_begin_cb
    ws_msg_frame_data_callback_f msg_frame_data_cb;
                                ///< Callback for when data
                                /// in a frame is received.
    void *msg_frame_data_arg;   ///< User supplied argument for
                                /// the ws_s#msg_frame_data_cb.
    ws_msg_frame_end_callback_f msg_frame_end_cb;
                                ///< Callback for when a frame ends.
    void *msg_frame_end_arg;    ///< User supplied argument for
                                /// the ws_s#msg_frame_end_cb.
    ws_close_callback_f close_cb;
                                ///< Callback for when the websocket connection
                                /// is closed.
    void *close_arg;            ///< The user supplied argument
                                /// to pass to the ws_s#close_cb
                                /// callback.

    ///
    /// @defgroup ConnectionCallback Connection callback
    /// @{
    ///
    ws_connect_callback_f connect_cb;
                                ///< Callback for when the
                                /// connection is complete.
    void *connect_arg;          ///< The user supplied argument
    ws_timeout_callback_f connect_timeout_cb;
                                ///< Connection timeout callback.
    struct timeval connect_timeout;
                                ///< Connection timeout.
    void *connect_timeout_arg;  ///< The user supplied argument that is passed
                                /// to the ws_s#connect_timeout_cb callback.
    ws_timer connect_timeout_event;
                                ///< Libevent event that is fired when the
                                /// connection times out.
    /// @}

    ws_timeout_callback_f recv_timeout_cb;
    struct timeval recv_timeout;
    void *recv_timeout_arg;

    ws_timeout_callback_f send_timeout_cb;
    struct timeval send_timeout;
    void *send_timeout_arg;

    ///
    /// @defgroup PongCallback Pong callback
    /// @{
    ///
    ws_msg_callback_f pong_cb;  ///< User supplied callback for
                                /// when a pong frame is received.
    void *pong_arg;             ///< The user supplied argument that will be 
                                /// passed to the pong callback.
    ws_timeout_callback_f pong_timeout_cb;
    void *pong_timeout_arg;
    struct timeval pong_timeout;
    ws_timer pong_timeout_event;
    /// @}

    ///
    /// @defgroup PingCallback Ping callback
    /// @{
    ///
    ws_msg_callback_f ping_cb;
    void *ping_arg;
    /// @}

    ws_header_callback_f header_cb;
    void *header_arg;
    ws_http_header_flags_t http_header_flags;
    /// @}

    ///
    /// @defgroup ConnectionVariables    Connection variables
    /// @{
    ///
    char *server;
    char *uri;
    int port;
    char *handshake_key_base64;
    char *origin;
    char **subprotocols;
    size_t num_subprotocols;
    /// @}

    int binary_mode;            ///< If this is set messages
                                /// will be sent as binary.
    int debug_level;
    uint64_t max_frame_size;    ///< The max frame size to allow before chunking.

    ///
    /// @defgroup FrameReceive Frame receive variables
    /// @{
    ///
    struct evbuffer *msg;       ///< Buffer that is used to
                                /// build an incoming message.
    struct evbuffer *frame_data;///< Data for the current frame.
    uint64_t recv_frame_len;    ///< The amount of bytes that have been read
                                /// for the current frame so far.
    int has_header;             ///< Has the websocket header been read yet?
    ws_header_t header;         ///< Header for received websocket frame.
    int in_msg;                 ///< Are we inside a message?
    int msg_isbinary;           ///< The opcode of the current message.
    ws_utf8_state_t utf8_state; ///< Current state of utf8 validator.
    char ctrl_payload[WS_CONTROL_MAX_PAYLOAD_LEN];
                                ///< Control frame payload.
    size_t ctrl_len;            ///< Length of the control payload.
    int received_close;         ///< Did we receive a close frame?
    int sent_close;             ///< Have we sent a close frame?
    ws_timer close_timeout_event;
                                ///< Timeout even for waiting for a close reply.
    ws_close_status_t server_close_status; 
                                ///< The Close status the server sent.
    char *server_reason;        ///< Server close reason data.
    size_t server_reason_len;   ///< Server close reason length.
    /// @}
    
    ///
    /// @defgroup FrameSend Frame send variables
    /// @{
    ///
    uint64_t frame_size;        ///< The frame size of the frame
                                /// currently being sent.
    uint64_t frame_data_sent;   ///< The number of bytes sent so
                                /// far of the current frame.
    ws_send_state_t send_state; ///< The state for sending data.
    ws_no_copy_cleanup_f no_copy_cleanup_cb;
                                ///< If set, any data written to
                                /// the websocket will be freed 
                                /// using this callback.
    void *no_copy_extra;        ///< User supplied argument for
                                /// the ws_s#no_copy_cleanup_cb
    /// @}

    struct ev_token_bucket_cfg *rate_limits;
                                ///< Rate limits.
    #ifdef LIBWS_WITH_OPENSSL
    ///
    /// @defgroup OpenSSL OpenSSL variables
    ///
    libws_ssl_state_t use_ssl;  ///< If SSL should be used or not.
    SSL_CTX *ssl_ctx;
    SSL *ssl;                   ///< SSL session.
    #endif // LIBWS_WITH_OPENSSL
} ws_s;

///
/// Creates a timeout timer
///
int _ws_setup_timeout_event(ws_t ws, event_callback_fn func, ws_timer* timer, struct timeval *tv);

///
/// Frees and NULLs an timer - asynchronously (i.e. just marks it as canceled - when it triggers,
/// the callback deletes it) if LIBWS_EXTERNAL_LOOP is enabled,
/// and synchronously otherwise.
void _ws_free_timer(ws_timer *timer);

/// Synchronously frees a timer - always performs the actual deletion immediately. Not to be used
/// directly on a registered timer when LIBWS_EXTERNAL_LOOP, as if the timer has already fired and
/// its message is queued on the marshalled, when its received the timer struct will be already freed
void _ws_do_free_timer(ws_timer* timer);
///
/// Creates a timeout event for when connecting.
///
/// @param[in] ws   The websocket context.
///
/// @returns        0 on success.
///
int _ws_setup_connection_timeout(ws_t ws);

///
/// Creates a timeout event for an expected pong reply.
///
/// @param[in] ws   The websocket context.
///
/// @returns        0 on success.
///
int _ws_setup_pong_timeout(ws_t ws);

/// 
/// Creates the libevent bufferevent socket.
///
/// @param[in] ws   The websocket context.
///
/// @returns        0 on success.
///
int _ws_create_bufferevent_socket(ws_t ws);

///
/// Sends data over the bufferevent socket.
///
/// @param[in] ws      The websocket context.
/// @param[in] msg     The buffer to send.
/// @param[in] len     Length of the buffer to send.
/// @param[in] no_copy Should we copy the contents of the buffer
///                    or simply use a reference? Used for zero copy.
///                    This requires that the buffer is still valid
///                    until the bufferevent sends it. To enable this
///                    you have to set a cleanup function using
///                    #ws_set_no_copy_cb that frees the data.
///
/// @returns            0 on success.
/// 
int _ws_send_data(ws_t ws, char *msg, uint64_t len, int no_copy);

///
/// Sends a raw websocket frame.
///
/// @param[in] ws       The websocket context.
/// @param[in] opcode   The websocket operation code.
/// @param[in] data     Application data for the websocket frame.
/// @param[in] datalen  Length of the data.
///
/// @returns            0 on success.
///
int _ws_send_frame_raw(ws_t ws, ws_opcode_t opcode, 
                        char *data, uint64_t datalen);

///
/// Sends a close frame.
///
/// @param[in] ws          The websocket context.
/// @param[in] status_code The status code to send.
/// @param[in] reason      The reason data.
/// @param[in] reason_len  The length of the #reason buffer.
///                        Must not be larger than 123 bytes.
///
/// @returns               0 on success.
///
int _ws_send_close(ws_t ws, ws_close_status_t status_code, 
                    const char *reason, size_t reason_len);

///
/// Closes the socket for the underlying TCP session used for the websocket.
///
/// @param[in] ws         The websocket context.
///
void _ws_shutdown(ws_t ws);

///
/// Timeout callback function for when we have sent the close frame to the
/// server. If this times out, we will initiate an unclean shutdown since
/// the servern hasn't initiated the TCP close.
///
void _ws_close_timeout_cb(evutil_socket_t fd, short what, void *arg);

///
/// Randomizes the contents of #buf. This is used for generating
/// the 32-bit payload mask.
///
/// @param[in] ws      The websocket context.
/// @param[in] buf     The buffer to randomize.
/// @param[in] len     Size of #buf.
///
/// @returns           The number of bytes successfully randomized.
///                    Negative on error.
///
int _ws_get_random_mask(ws_t ws, char *buf, size_t len);

///
/// Sets timeouts for read and write events of the underlying bufferevent.
///
/// @param[in] ws      The websocket context.
/// 
void _ws_set_timeouts(ws_t ws);

///
/// Replacement malloc.
///
void *_ws_malloc(size_t size);

///
/// Replacement realloc.
///
void *_ws_realloc(void *ptr, size_t size);

///
/// Replacement free.
///
void _ws_free(void *ptr);

///
/// Replacement calloc.
///
void *_ws_calloc(size_t count, size_t size);

///
/// Replacement strdup.
///
char *_ws_strdup(const char *str);

///
/// Internal function to replace memory functions.
///
void _ws_set_memory_functions(ws_malloc_replacement_f malloc_replace,
                             ws_free_replacement_f free_replace,
                             ws_realloc_replacement_f realloc_replace);


#endif // __LIBWS_PRIVATE_H__
