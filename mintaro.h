// Small retro game framework. Public domain. See "unlicense" statement at the end of this file.
// mintaro - v0.1 - Release Date TBD
//
// David Reid - mackron@gmail.com

// USAGE
// =====
// Mintaro is a single-file library. To use it, do something like the following in one .c file.
//   #define MINTARO_IMPLEMENTATION
//   #include "mintaro.h"
//
// You can then #include this file in other parts of the program as you would with any other header file.
//
//
// Building (Windows)
// ------------------
// Linking (MSVC): Nothing
// Linking (GCC/Clang): -lgdi32
//
// You will need dsound.h in your include paths, but linking to dsound.lib is unnecessary.
//
//
// Building (Linux)
// ----------------
// The Linux build uses ALSA for it's backend so you will need to install the relevant ALSA development pacakges
// for your preferred distro. It also uses X11 and pthreads, both of which should be easy to set up.
//
// Linking: -lX11 -lXext -lasound -lpthread -lm
//
//
//
// NOTES
// =====
// - This embeds mini_al for audio. See https://github.com/dr-soft/mini_al for more information.
// - Mintaro is not currently thread-safe.
//
// Graphics
// --------
// - The number of colors in the palette is configurable at initialization time, but has a maximum of 256 colors, with
//   one color designated as transparency (there is only 1 level of transparency).
// - The color index to use for transparency is configurable in case you want to plug in an existing palette.
//
// Audio
// -----
// - Optimal audio format: Stereo, 44100Hz, 16-bit signed integer PCM.
// - 44100Hz is the only supported sample rate. Sounds will always be played at this rate regardless of the sample
//   rate of the source file.

#ifndef mintaro_h
#define mintaro_h

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define MO_WIN32
#include <windows.h>
#else
#define MO_X11
#define MO_POSIX
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XShm.h>
#endif

///////////////////////////////////////////////////////////////////////////////
//
// BEGIN MINI_AL HEADER SECTTION
//
///////////////////////////////////////////////////////////////////////////////
#if MO_USE_EXTERNAL_MINI_AL
#include "../mini_al/mini_al.h"
#else
#ifdef _WIN32
    #define MAL_WIN32
#else
    #define MAL_POSIX
    #include <pthread.h>    // Unfortunate #include, but needed for pthread_t, pthread_mutex_t and pthread_cond_t types.

    #ifdef __linux__
        #define MAL_LINUX
    #endif
    #ifdef __ANDROID__
        #define MAL_ANDROID
    #endif
#endif

#if !defined(MAL_NO_DSOUND) && defined(MAL_WIN32)
    #define MAL_ENABLE_DSOUND
#endif
#if !defined(MAL_NO_ALSA) && defined(MAL_LINUX) && !defined(MAL_ANDROID)
    #define MAL_ENABLE_ALSA
#endif
#if !defined(MAL_NO_OPENSLES) && defined(MAL_ANDROID)
    #define MAL_ENABLE_OPENSLES
#endif
#if !defined(MAL_NO_NULL)
    #define MAL_ENABLE_NULL
#endif


#if defined(_MSC_VER) && _MSC_VER < 1600
typedef   signed char    mal_int8;
typedef unsigned char    mal_uint8;
typedef   signed short   mal_int16;
typedef unsigned short   mal_uint16;
typedef   signed int     mal_int32;
typedef unsigned int     mal_uint32;
typedef   signed __int64 mal_int64;
typedef unsigned __int64 mal_uint64;
#else
#include <stdint.h>
typedef int8_t           mal_int8;
typedef uint8_t          mal_uint8;
typedef int16_t          mal_int16;
typedef uint16_t         mal_uint16;
typedef int32_t          mal_int32;
typedef uint32_t         mal_uint32;
typedef int64_t          mal_int64;
typedef uint64_t         mal_uint64;
#endif
typedef mal_int8         mal_bool8;
typedef mal_int32        mal_bool32;
#define MAL_TRUE         1
#define MAL_FALSE        0

typedef void* mal_handle;
typedef void* mal_ptr;

#ifdef MAL_WIN32
    typedef mal_handle mal_thread;
    typedef mal_handle mal_mutex;
    typedef mal_handle mal_event;
#else
    typedef pthread_t mal_thread;
    typedef pthread_mutex_t mal_mutex;
    typedef struct
    {
        pthread_mutex_t mutex;
        pthread_cond_t condition;
        mal_uint32 value;
    } mal_event;
#endif

#ifdef MAL_ENABLE_DSOUND
    #define MAL_MAX_PERIODS_DSOUND    4
#endif

typedef int mal_result;
#define MAL_SUCCESS                              0
#define MAL_ERROR                               -1      // A generic error.
#define MAL_INVALID_ARGS                        -2
#define MAL_OUT_OF_MEMORY                       -3
#define MAL_FORMAT_NOT_SUPPORTED                -4
#define MAL_NO_BACKEND                          -5
#define MAL_NO_DEVICE                           -6
#define MAL_API_NOT_FOUND                       -7
#define MAL_DEVICE_BUSY                         -8
#define MAL_DEVICE_NOT_INITIALIZED              -9
#define MAL_DEVICE_ALREADY_STARTED              -10
#define MAL_DEVICE_ALREADY_STARTING             -11
#define MAL_DEVICE_ALREADY_STOPPED              -12
#define MAL_DEVICE_ALREADY_STOPPING             -13
#define MAL_FAILED_TO_MAP_DEVICE_BUFFER         -14
#define MAL_FAILED_TO_INIT_BACKEND              -15
#define MAL_FAILED_TO_READ_DATA_FROM_CLIENT     -16
#define MAL_FAILED_TO_START_BACKEND_DEVICE      -17
#define MAL_FAILED_TO_STOP_BACKEND_DEVICE       -18
#define MAL_FAILED_TO_CREATE_MUTEX              -19
#define MAL_FAILED_TO_CREATE_EVENT              -20
#define MAL_FAILED_TO_CREATE_THREAD             -21
#define MAL_DSOUND_FAILED_TO_CREATE_DEVICE      -1024
#define MAL_DSOUND_FAILED_TO_SET_COOP_LEVEL     -1025
#define MAL_DSOUND_FAILED_TO_CREATE_BUFFER      -1026
#define MAL_DSOUND_FAILED_TO_QUERY_INTERFACE    -1027
#define MAL_DSOUND_FAILED_TO_SET_NOTIFICATIONS  -1028
#define MAL_ALSA_FAILED_TO_OPEN_DEVICE          -2048
#define MAL_ALSA_FAILED_TO_SET_HW_PARAMS        -2049
#define MAL_ALSA_FAILED_TO_SET_SW_PARAMS        -2050

typedef struct mal_device mal_device;

typedef void       (* mal_recv_proc)(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples);
typedef mal_uint32 (* mal_send_proc)(mal_device* pDevice, mal_uint32 frameCount, void* pSamples);
typedef void       (* mal_stop_proc)(mal_device* pDevice);
typedef void       (* mal_log_proc) (mal_device* pDevice, const char* message);

typedef enum
{
    mal_api_null,
    mal_api_dsound,
    mal_api_alsa,
    mal_api_sles
} mal_api;

typedef enum
{
    mal_device_type_playback,
    mal_device_type_capture
} mal_device_type;

typedef enum
{
    // I like to keep these explicitly defined because they're used as a key into a lookup table. When items are
    // added to this, make sure there are no gaps and that they're added to the lookup table in mal_get_sample_size_in_bytes().
    mal_format_u8  = 0,
    mal_format_s16 = 1,   // Seems to be the most widely supported format.
    mal_format_s24 = 2,   // Tightly packed. 3 bytes per sample.
    mal_format_s32 = 3,
    mal_format_f32 = 4,
} mal_format;

typedef union
{
    mal_uint32 id32;    // OpenSL|ES uses a 32-bit unsigned integer for identification.
    char str[32];       // ALSA uses a name string for identification.
    mal_uint8 guid[16]; // DirectSound uses a GUID for identification.
} mal_device_id;

typedef struct
{
    mal_device_id id;
    char name[256];
} mal_device_info;

typedef struct
{
    int64_t counter;
} mal_timer;

typedef struct
{
    mal_format format;
    mal_uint32 channels;
    mal_uint32 sampleRate;
    mal_uint32 bufferSizeInFrames;
    mal_uint32 periods;
    mal_recv_proc onRecvCallback;
    mal_send_proc onSendCallback;
    mal_stop_proc onStopCallback;
    mal_log_proc  onLogCallback;
} mal_device_config;

struct mal_device
{
    mal_api api;            // DirectSound, ALSA, etc.
    mal_device_type type;
    mal_format format;
    mal_uint32 channels;
    mal_uint32 sampleRate;
    mal_uint32 bufferSizeInFrames;
    mal_uint32 periods;
    mal_uint32 state;
    mal_recv_proc onRecv;
    mal_send_proc onSend;
    mal_stop_proc onStop;
    mal_log_proc onLog;
    void* pUserData;        // Application defined data.
    mal_mutex lock;
    mal_event wakeupEvent;
    mal_event startEvent;
    mal_event stopEvent;
    mal_thread thread;
    mal_result workResult;  // This is set by the worker thread after it's finished doing a job.
    mal_uint32 flags;       // MAL_DEVICE_FLAG_*

    union
    {
    #ifdef MAL_ENABLE_DSOUND
        struct
        {
            /*HMODULE*/ mal_handle hDSoundDLL;
            /*LPDIRECTSOUND8*/ mal_ptr pPlayback;
            /*LPDIRECTSOUNDBUFFER*/ mal_ptr pPlaybackPrimaryBuffer;
            /*LPDIRECTSOUNDBUFFER*/ mal_ptr pPlaybackBuffer;
            /*LPDIRECTSOUNDCAPTURE8*/ mal_ptr pCapture;
            /*LPDIRECTSOUNDCAPTUREBUFFER8*/ mal_ptr pCaptureBuffer;
            /*LPDIRECTSOUNDNOTIFY*/ mal_ptr pNotify;
            /*HANDLE*/ mal_handle pNotifyEvents[MAL_MAX_PERIODS_DSOUND];  // One event handle for each period.
            /*HANDLE*/ mal_handle hStopEvent;
            /*HANDLE*/ mal_handle hRewindEvent;
            mal_uint32 lastProcessedFrame;      // This is circular.
            mal_uint32 rewindTarget;            // Where we want to rewind to. Set to ~0UL when it is not being rewound.
            mal_bool32 breakFromMainLoop;
        } dsound;
    #endif

    #ifdef MAL_ENABLE_ALSA
        struct
        {
            /*snd_pcm_t**/ mal_ptr pPCM;
            mal_bool32 isUsingMMap;
            mal_bool32 breakFromMainLoop;
            void* pIntermediaryBuffer;
        } alsa;
    #endif

    #ifdef MAL_ENABLE_OPENSLES
        struct
        {
            /*SLObjectItf*/ mal_ptr pOutputMixObj;
            /*SLOutputMixItf*/ mal_ptr pOutputMix;
            /*SLObjectItf*/ mal_ptr pAudioPlayerObj;
            /*SLPlayItf*/ mal_ptr pAudioPlayer;
            /*SLObjectItf*/ mal_ptr pAudioRecorderObj;
            /*SLRecordItf*/ mal_ptr pAudioRecorder;
            /*SLAndroidSimpleBufferQueueItf*/ mal_ptr pBufferQueue;
            mal_uint32 periodSizeInFrames;
            mal_uint32 currentBufferIndex;
            mal_uint8* pBuffer;                 // This is malloc()'d and is used for storing audio data. Typed as mal_uint8 for easy offsetting.
        } sles;
    #endif

    #ifdef MAL_ENABLE_NULL
        struct
        {
            mal_timer timer;
            mal_uint32 lastProcessedFrame;      // This is circular.
            mal_bool32 breakFromMainLoop;
            mal_uint8* pBuffer;                 // This is malloc()'d and is used as the destination for reading from the client. Typed as mal_uint8 for easy offsetting.
        } null_device;
    #endif
    };
};

mal_result mal_enumerate_devices(mal_device_type type, mal_uint32* pCount, mal_device_info* pInfo);
mal_result mal_device_init(mal_device* pDevice, mal_device_type type, mal_device_id* pDeviceID, mal_device_config* pConfig, void* pUserData);
void mal_device_uninit(mal_device* pDevice);
void mal_device_set_recv_callback(mal_device* pDevice, mal_recv_proc proc);
void mal_device_set_send_callback(mal_device* pDevice, mal_send_proc proc);
void mal_device_set_stop_callback(mal_device* pDevice, mal_stop_proc proc);
mal_result mal_device_start(mal_device* pDevice);
mal_result mal_device_stop(mal_device* pDevice);
mal_bool32 mal_device_is_started(mal_device* pDevice);
mal_uint32 mal_device_get_available_rewind_amount(mal_device* pDevice);
mal_uint32 mal_device_rewind(mal_device* pDevice, mal_uint32 framesToRewind);
mal_uint32 mal_device_get_buffer_size_in_bytes(mal_device* pDevice);
mal_uint32 mal_get_sample_size_in_bytes(mal_format format);
#endif
///////////////////////////////////////////////////////////////////////////////
//
// END MINI_AL HEADER SECTTION
//
///////////////////////////////////////////////////////////////////////////////


#define MO_GLYPH_SIZE		        9

typedef int mo_result;
#define MO_SUCCESS		 	 		 0
#define MO_ERROR				    -1  // A generic error.
#define MO_INVALID_ARGS				-2
#define MO_OUT_OF_MEMORY			-3
#define MO_FAILED_TO_INIT_PLATFORM	-4
#define MO_DOES_NOT_EXIST			-5
#define MO_INVALID_RESOURCE			-6
#define MO_UNSUPPORTED_IMAGE_FORMAT	-7
#define MO_FAILED_TO_INIT_AUDIO     -8
#define MO_BAD_PROFILE              -9

typedef unsigned int mo_event_type;
#define MO_EVENT_TYPE_KEY_DOWN		1
#define MO_EVENT_TYPE_KEY_UP		2

typedef unsigned int mo_button;
#define MO_BUTTON_LEFT				(1 << 0)
#define MO_BUTTON_UP				(1 << 1)
#define MO_BUTTON_RIGHT				(1 << 2)
#define MO_BUTTON_DOWN				(1 << 3)
#define MO_BUTTON_A					(1 << 4)
#define MO_BUTTON_B					(1 << 5)
#define MO_BUTTON_SELECT			(1 << 6)
#define MO_BUTTON_START				(1 << 7)
#define MO_BUTTON_COUNT             8

typedef unsigned int mo_key;
#define MO_KEY_BACKSPACE            0xff08
#define MO_KEY_ENTER                0xff0d
#define MO_KEY_SHIFT                0xff10
#define MO_KEY_ESCAPE               0xff1b
#define MO_KEY_SPACE                0xff20
#define MO_KEY_PAGE_UP              0xff55
#define MO_KEY_PAGE_DOWN            0xff56
#define MO_KEY_END                  0xff57
#define MO_KEY_HOME                 0xff50
#define MO_KEY_ARROW_LEFT           0x08fb
#define MO_KEY_ARROW_UP             0x08fc
#define MO_KEY_ARROW_RIGHT          0x08fd
#define MO_KEY_ARROW_DOWN           0x08fe
#define MO_KEY_DELETE               0xffff
#define MO_KEY_F1                   0xffbe
#define MO_KEY_F2                   0xffbf
#define MO_KEY_F3                   0xffc0
#define MO_KEY_F4                   0xffc1
#define MO_KEY_F5                   0xffc2
#define MO_KEY_F6                   0xffc3
#define MO_KEY_F7                   0xffc4
#define MO_KEY_F8                   0xffc5
#define MO_KEY_F9                   0xffc6
#define MO_KEY_F10                  0xffc7
#define MO_KEY_F11                  0xffc8
#define MO_KEY_F12                  0xffc9

#define MO_SOUND_GROUP_MASTER       0
#define MO_SOUND_GROUP_EFFECTS      1
#define MO_SOUND_GROUP_MUSIC        2
#define MO_SOUND_GROUP_VOICE        3
#define MO_SOUND_GROUP_COUNT        4

#if defined(_MSC_VER) && _MSC_VER < 1300
typedef   signed char       mo_int8;
typedef unsigned char       mo_uint8;
typedef   signed short      mo_int16;
typedef unsigned short      mo_uint16;
typedef   signed int        mo_int32;
typedef unsigned int        mo_uint32;
typedef   signed __int64    mo_int64;
typedef unsigned __int64    mo_uint64;
#else
#include <stdint.h>
typedef int8_t              mo_int8;
typedef uint8_t             mo_uint8;
typedef int16_t             mo_int16;
typedef uint16_t            mo_uint16;
typedef int32_t             mo_int32;
typedef uint32_t            mo_uint32;
typedef int64_t             mo_int64;
typedef uint64_t            mo_uint64;
#endif
typedef mo_int8             mo_bool8;
typedef mo_int32            mo_bool32;
typedef mo_uint8            mo_color_index;
#define MO_TRUE             1
#define MO_FALSE            0

typedef struct mo_context mo_context;
typedef struct mo_sound_source mo_sound_source;
typedef struct mo_sound_group mo_sound_group;
typedef struct mo_sound mo_sound;

typedef enum
{
	mo_image_format_unknown = 0,
	mo_image_format_native,
	mo_image_format_rgba8
} mo_image_format;

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4201)
#endif
typedef union
{
	struct
	{
		mo_uint8 b;
		mo_uint8 g;
		mo_uint8 r;
		mo_uint8 a;
	};
	
	mo_uint32 rgba;
} mo_color_rgba;
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

typedef struct
{
    float y;
    float u;
    float v;
} mo_color_yuv;

typedef struct
{
    mo_uint32 resolutionX;   // The width of the virtual screen.
    mo_uint32 resolutionY;   // The height of the virtual screen.
    mo_color_index transparentColorIndex;   // The index of the color in the palette representing transparency.
	mo_uint32 paletteSize;   // The number of available colors. Maximum of 256.
    mo_color_rgba palette[256]; // Palette colors.
} mo_profile;

typedef struct
{
	mo_uint32 width;
	mo_uint32 height;
	mo_image_format format;
	mo_uint8 pData[1];
} mo_image;

struct mo_sound_source
{
    mo_uint32 channels;
    mo_uint32 sampleRate;
    mo_uint64 sampleCount;
    mo_int16 pSampleData[1];
};

struct mo_sound
{
    mo_context* pContext;
    mo_sound_source* pSource;
    mo_uint32 group;
    float linearVolume;
    float pan;
    mo_uint32 flags;
    mo_uint64 currentSample;
    mo_bool32 isMarkedForDeletion;
};

struct mo_sound_group
{
    float linearVolume;
    mo_uint32 flags;
};


typedef struct
{
    int64_t counter;
} mo_timer;

// Initializes a high-resolution timer.
void mo_timer_init(mo_timer* pTimer);

// Ticks the timer and returns the number of seconds since the previous tick.
//
// The maximum return value is about 140 years or so.
double mo_timer_tick(mo_timer* pTimer);


typedef void (* mo_on_step_proc)(mo_context* pContext, double dt);
typedef void (* mo_on_log_proc) (mo_context* pContext, const char* message);

struct mo_context
{
	mo_on_step_proc onStep;
    mo_on_log_proc onLog;
	void* pUserData;

    // The profile used to initialize the context. This defines things like the screen resolution
    // and the palette.
    mo_profile profile;

	// The pixel data of the virtual screen. Each pixel is reprsented with a single byte which is
    // an index into the palette.
    mo_color_index* screen;
	
	// Button state. A set bit means the key is down.
	unsigned int buttonState;
	unsigned int buttonPressState;
	unsigned int buttonReleaseState;

    // Key bindings.
    mo_key keymap[MO_BUTTON_COUNT];
	
	// Timer.
	mo_timer timer;
	
	// Boolean flags;
	mo_uint32 flags;
	
	
	// The platform-specific window.
#ifdef MO_WIN32
	HWND hWnd;
    HDC hDC;
    int windowWidth;
    int windowHeight;
    HDC hDIBDC;
    HBITMAP hDIBSection;
    void* pScreenRGBA_DIB;
#endif
#ifdef MO_X11
	Window windowX11;
	GC gcX11;
	XImage* pPresentBufferX11;
	XShmSegmentInfo shmInfo;	// Only used if MIT-SHM is supported.
#endif

    // Audio
#ifdef MO_ENABLE_DSOUND
    HMODULE hDSoundDLL;
    LPDIRECTSOUND8 pDS;
    LPDIRECTSOUNDBUFFER pDSPrimaryBuffer;
    LPDIRECTSOUNDBUFFER pDSSecondaryBuffer;
#endif
#ifdef MO_ENABLE_ALSA

#endif

    // The playback device for audio. This always uses the default device for now.
    mal_device playbackDevice2;

    // Sound groups. There's a fixed number of groups, and they are referenced with an index.
    mo_sound_group soundGroups[MO_SOUND_GROUP_COUNT];

    // TODO: Improve memory management for sounds.
    //
    // Sounds. This is just a basic realloc()'ed array of mo_sound* pointers.
    mo_sound** ppSounds;
    mo_uint32 soundCount;
    mo_uint32 soundBufferSize;

    // Keeps track of whether or not there is at least one sound needing to be deleted at the end
    // of the next step. This is used for garbage collection of sounds.
    mo_bool32 isSoundMarkedForDeletion;



    // Dynamically sized data.
    mo_uint8 pExtraData[1];
};

// Initializes a context. pProfile can be null, in which case it defaults to 160x144, with a 255 color general palette.
mo_result mo_init(mo_profile* pProfile, mo_uint32 windowSizeX, mo_uint32 windowSizeY, const char* title, mo_on_step_proc onStep, void* pUserData, mo_context** ppContext);

// Uninitializes a context.
void mo_uninit(mo_context* pContext);

// Runs the game. Call mo_close() to exit the main loop.
int mo_run(mo_context* pContext);

// Exits the game's main loop. This does not uninitialize the context.
void mo_close(mo_context* pContext);

// Posts a log message.
void mo_log(mo_context* pContext, const char* message);
void mo_logf(mo_context* pContext, const char* format, ...);


//// Resources ////

// Creates an image from raw image data.
mo_result mo_image_create(mo_context* pContext, unsigned int width, unsigned int height, mo_image_format format, const void* pData, mo_image** ppImage);

// Loads an image. The image can be unloaded with mo_delete_image().
mo_result mo_image_load(mo_context* pContext, const char* filePath, mo_image** ppImage);

// Deletes an image.
void mo_image_delete(mo_context* pContext, mo_image* pImage);


//// Drawing ////

// Finds the color index for the given RGBA color code.
mo_color_index mo_find_closest_color(mo_context* pContext, mo_color_rgba color);

// Clears the screen.
void mo_clear(mo_context* pContext, mo_color_index colorIndex);

// Draws a quad.
void mo_draw_quad(mo_context* pContext, int posX, int posY, int sizeX, int sizeY, mo_color_index colorIndex);

// Draws a string of text.
void mo_draw_text(mo_context* pContext, int posX, int posY, mo_color_index colorIndex, const char* text);
void mo_draw_textf(mo_context* pContext, int posX, int posY, mo_color_index colorIndex, const char* format, ...);

// Draws an image.
void mo_draw_image(mo_context* pContext, int dstX, int dstY, mo_image* pImage, int srcX, int srcY, int srcWidth, int srcHeight);

// Draws an image with scaling.
void mo_draw_image_scaled(mo_context* pContext, int dstX, int dstY, int dstWidth, int dstHeight, mo_image* pImage, int srcX, int srcY, int srcWidth, int srcHeight/*, float rotation*/);


//// Audio ////

// Creates a sound source. When a sound is played, you pass in a reference to this source.
mo_result mo_sound_source_create(mo_context* pContext, unsigned int channels, unsigned int sampleRate, mo_uint64 sampleCount, const mo_int16* pSampleData, mo_sound_source** ppSource);

// Loads a sound source from a file.
mo_result mo_sound_source_load(mo_context* pContext, const char* filePath, mo_sound_source** ppSource);

// Deletes a sound source.
void mo_sound_source_delete(mo_sound_source* pSource);

// Helper function for creating a sound tied to the given sound source, play it, and then delete it once it's
// finished playing. The sound does not loop.
mo_result mo_play_sound_source(mo_context* pContext, mo_sound_source* pSource, mo_uint32 group);


// Pauses playback of all sounds in the given sound group.
void mo_sound_group_pause(mo_context* pContext, mo_uint32 group);

// Resumes playback of all sounds in the given sound group.
void mo_sound_group_resume(mo_context* pContext, mo_uint32 group);

// Determines whether or not the given group is paused.
mo_bool32 mo_sound_group_is_paused(mo_context* pContext, mo_uint32 group);

// Sets the volume of the group. This is modulated with the volumes of each individual sound.
void mo_sound_group_set_volume(mo_context* pContext, mo_uint32, float linearVolume);


// Creates a sound. The sound group can be null in which case it'll be added to the global group.
//
// The <group> parameter should be one of the following:
//   - 0 (same as MO_SOUND_GROUP_MASTER)
//   - MO_SOUND_GROUP_MASTER
//   - MO_SOUND_GROUP_EFFECTS
//   - MO_SOUND_GROUP_MUSIC
//   - MO_SOUND_GROUP_VOICE
mo_result mo_sound_create(mo_context* pContext, mo_sound_source* pSource, mo_uint32 group, mo_sound** ppSound);

// Deletes a sound.
void mo_sound_delete(mo_sound* pSound);

// Marks a sound for deletion. The sound will be deleted for real at the end of the next step.
void mo_sound_mark_for_deletion(mo_sound* pSound);

// Sets the volume of the given sound. The volume is linear.
void mo_sound_set_volume(mo_sound* pSound, float linearVolume);

// Plays the given sound.
void mo_sound_play(mo_sound* pSound, mo_bool32 loop);

// Stops playback of the given sound.
void mo_sound_stop(mo_sound* pSound);

// Determines whether or not the given sound is playing.
mo_bool32 mo_sound_is_playing(mo_sound* pSound);

// Determines whether or not the given sound is looping.
mo_bool32 mo_sound_is_looping(mo_sound* pSound);



//// Input ////

// Binds a key to a button.
void mo_bind_key_to_button(mo_context* pContext, mo_key key, mo_button button);

// Retrieves the button bound to the given key. Returns 0 if the key is not bound to any button.
mo_button mo_get_key_binding(mo_context* pContext, mo_key key);

// Determines if a button is currently down.
mo_bool32 mo_is_button_down(mo_context* pContext, unsigned int button);

// Determines if a button has just been pressed.
mo_bool32 mo_was_button_pressed(mo_context* pContext, unsigned int button);

// Determines if a button has just been released.
mo_bool32 mo_was_button_released(mo_context* pContext, unsigned int button);


//// Misc ////
#define mo_degrees(radians) ((radians) * 57.29577951308232087685f)
#define mo_radians(degrees) ((degrees) *  0.01745329251994329577f)

#ifdef __cplusplus
}
#endif
#endif  //dr_gbe_h

///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MINTARO_IMPLEMENTATION
#include <assert.h>
#include <stdio.h>  // Required for printf() and family which is used in mo_logf().
#include <stdarg.h> // va_list, va_start, va_arg, va_end

// Standard library functions.
#ifndef mo_zero_memory
#ifdef _WIN32
#define mo_zero_memory(p, sz) ZeroMemory((p), (sz))
#else
#define mo_zero_memory(p, sz) memset((p), 0, (sz))
#endif
#endif

#ifndef mo_copy_memory
#ifdef _WIN32
#define mo_copy_memory(dst, src, sz) CopyMemory((dst), (src), (sz))
#else
#define mo_copy_memory(dst, src, sz) memcpy((dst), (src), (sz))
#endif
#endif

#ifndef mo_malloc
#ifdef _WIN32
#define mo_malloc(sz) HeapAlloc(GetProcessHeap(), 0, (sz))
#else
#define mo_malloc(sz) malloc((sz))
#endif
#endif

#ifndef mo_calloc
#ifdef _WIN32
#define mo_calloc(sz) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (sz))
#else
#define mo_calloc(sz) calloc(1, (sz))
#endif
#endif

#ifndef mo_realloc
#ifdef _WIN32
// HeapReAlloc() has slightly different behaviour to realloc().
#define mo_realloc(p, sz) (((sz) > 0) ? ((p) ? HeapReAlloc(GetProcessHeap(), 0, (p), (sz)) : HeapAlloc(GetProcessHeap(), 0, (sz))) : ((VOID*)(SIZE_T)(HeapFree(GetProcessHeap(), 0, (p)) & 0)))
#else
#define mo_realloc(p, sz) realloc((p), (sz))
#endif
#endif

#ifndef mo_free
#ifdef _WIN32
#define mo_free(p) HeapFree(GetProcessHeap(), 0, (p))
#else
#define mo_free(p) free((p))
#endif
#endif

#define mo_zero_object(p) mo_zero_memory((p), sizeof(*(p)))

#ifndef mo_assert
#ifdef _WIN32
#define mo_assert(condition) assert(condition)
#else
#define mo_assert(condition) assert(condition)
#endif
#endif



// External library support.
#ifdef STBI_INCLUDE_STB_IMAGE_H
#define MO_HAS_STB_IMAGE
#endif
#ifdef STB_VORBIS_INCLUDE_STB_VORBIS_H
#define MO_HAS_STB_VORBIS
#endif
#ifdef dr_flac_h
#define MO_HAS_DR_FLAC
#endif

#ifdef MO_X11
#include <stdlib.h>
#include <string.h>	// For memset()
#include <float.h>  // <-- What's this one for?
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

// Atomics.
#if defined(_WIN32) && defined(_MSC_VER)
#define mo_atomic_increment(a) InterlockedIncrement((LONG*)a)
#define mo_atomic_decrement(a) InterlockedDecrement((LONG*)a)
#else
#define mo_atomic_increment(a) __sync_add_and_fetch(a, 1)
#define mo_atomic_decrement(a) __sync_sub_and_fetch(a, 1)
#endif

#define MO_FLAG_CLOSING			            (1 << 0)
#define MO_FLAG_X11_USING_SHM	            (1 << 1)

#define MO_SOUND_GROUP_FLAG_PAUSED          (1 << 0)

#define MO_SOUND_FLAG_PLAYING               (1 << 0)
#define MO_SOUND_FLAG_PAUSED                (1 << 1)
#define MO_SOUND_FLAG_LOOPING               (1 << 2)
#define MO_SOUND_FLAG_STREAMING             (1 << 3)
#define MO_SOUND_FLAG_STOP_ON_NEXT_CHUNK    (1 << 4)
#define MO_SOUND_FLAG_INLINED               (1 << 5)    // Set when the sound was created by mo_play_sound_source().

static mo_uint32 g_moDefaultPalette[256] = {
    0xFF000000, 0xFF000033, 0xFF000066, 0xFF000099, 0xFF0000CC, 0xFF0000FF, 0xFF002B00, 0xFF002B33, 0xFF002B66, 0xFF002B99, 0xFF002BCC, 0xFF002BFF, 0xFF005500, 0xFF005533, 0xFF005566, 0xFF005599, 
    0xFF0055CC, 0xFF0055FF, 0xFF008000, 0xFF008033, 0xFF008066, 0xFF008099, 0xFF0080CC, 0xFF0080FF, 0xFF00AA00, 0xFF00AA33, 0xFF00AA66, 0xFF00AA99, 0xFF00AACC, 0xFF00AAFF, 0xFF00D500, 0xFF00D533, 
    0xFF00D566, 0xFF00D599, 0xFF00D5CC, 0xFF00D5FF, 0xFF00FF00, 0xFF00FF33, 0xFF00FF66, 0xFF00FF99, 0xFF00FFCC, 0xFF00FFFF, 0xFF330000, 0xFF330033, 0xFF330066, 0xFF330099, 0xFF3300CC, 0xFF3300FF, 
    0xFF332B00, 0xFF332B33, 0xFF332B66, 0xFF332B99, 0xFF332BCC, 0xFF332BFF, 0xFF335500, 0xFF335533, 0xFF335566, 0xFF335599, 0xFF3355CC, 0xFF3355FF, 0xFF338000, 0xFF338033, 0xFF338066, 0xFF338099, 
    0xFF3380CC, 0xFF3380FF, 0xFF33AA00, 0xFF33AA33, 0xFF33AA66, 0xFF33AA99, 0xFF33AACC, 0xFF33AAFF, 0xFF33D500, 0xFF33D533, 0xFF33D566, 0xFF33D599, 0xFF33D5CC, 0xFF33D5FF, 0xFF33FF00, 0xFF33FF33, 
    0xFF33FF66, 0xFF33FF99, 0xFF33FFCC, 0xFF33FFFF, 0xFF660000, 0xFF660033, 0xFF660066, 0xFF660099, 0xFF6600CC, 0xFF6600FF, 0xFF662B00, 0xFF662B33, 0xFF662B66, 0xFF662B99, 0xFF662BCC, 0xFF662BFF, 
    0xFF665500, 0xFF665533, 0xFF665566, 0xFF665599, 0xFF6655CC, 0xFF6655FF, 0xFF668000, 0xFF668033, 0xFF668066, 0xFF668099, 0xFF6680CC, 0xFF6680FF, 0xFF66AA00, 0xFF66AA33, 0xFF66AA66, 0xFF66AA99, 
    0xFF66AACC, 0xFF66AAFF, 0xFF66D500, 0xFF66D533, 0xFF66D566, 0xFF66D599, 0xFF66D5CC, 0xFF66D5FF, 0xFF66FF00, 0xFF66FF33, 0xFF66FF66, 0xFF66FF99, 0xFF66FFCC, 0xFF66FFFF, 0xFF990000, 0xFF990033, 
    0xFF990066, 0xFF990099, 0xFF9900CC, 0xFF9900FF, 0xFF992B00, 0xFF992B33, 0xFF992B66, 0xFF992B99, 0xFF992BCC, 0xFF992BFF, 0xFF995500, 0xFF995533, 0xFF995566, 0xFF995599, 0xFF9955CC, 0xFF9955FF, 
    0xFF998000, 0xFF998033, 0xFF998066, 0xFF998099, 0xFF9980CC, 0xFF9980FF, 0xFF99AA00, 0xFF99AA33, 0xFF99AA66, 0xFF99AA99, 0xFF99AACC, 0xFF99AAFF, 0xFF99D500, 0xFF99D533, 0xFF99D566, 0xFF99D599, 
    0xFF99D5CC, 0xFF99D5FF, 0xFF99FF00, 0xFF99FF33, 0xFF99FF66, 0xFF99FF99, 0xFF99FFCC, 0xFF99FFFF, 0xFFCC0000, 0xFFCC0033, 0xFFCC0066, 0xFFCC0099, 0xFFCC00CC, 0xFFCC00FF, 0xFFCC2B00, 0xFFCC2B33, 
    0xFFCC2B66, 0xFFCC2B99, 0xFFCC2BCC, 0xFFCC2BFF, 0xFFCC5500, 0xFFCC5533, 0xFFCC5566, 0xFFCC5599, 0xFFCC55CC, 0xFFCC55FF, 0xFFCC8000, 0xFFCC8033, 0xFFCC8066, 0xFFCC8099, 0xFFCC80CC, 0xFFCC80FF, 
    0xFFCCAA00, 0xFFCCAA33, 0xFFCCAA66, 0xFFCCAA99, 0xFFCCAACC, 0xFFCCAAFF, 0xFFCCD500, 0xFFCCD533, 0xFFCCD566, 0xFFCCD599, 0xFFCCD5CC, 0xFFCCD5FF, 0xFFCCFF00, 0xFFCCFF33, 0xFFCCFF66, 0xFFCCFF99, 
    0xFFCCFFCC, 0xFFCCFFFF, 0xFFFF0000, 0xFFFF0033, 0xFFFF0066, 0xFFFF0099, 0xFFFF00CC, 0xFFFF00FF, 0xFFFF2B00, 0xFFFF2B33, 0xFFFF2B66, 0xFFFF2B99, 0xFFFF2BCC, 0xFFFF2BFF, 0xFFFF5500, 0xFFFF5533, 
    0xFFFF5566, 0xFFFF5599, 0xFFFF55CC, 0xFFFF55FF, 0xFFFF8000, 0xFFFF8033, 0xFFFF8066, 0xFFFF8099, 0xFFFF80CC, 0xFFFF80FF, 0xFFFFAA00, 0xFFFFAA33, 0xFFFFAA66, 0xFFFFAA99, 0xFFFFAACC, 0xFFFFAAFF, 
    0xFFFFD500, 0xFFFFD533, 0xFFFFD566, 0xFFFFD599, 0xFFFFD5CC, 0xFFFFD5FF, 0xFFFFFF00, 0xFFFFFF33, 0xFFFFFF66, 0xFFFFFF99, 0xFFFFFFCC, 0xFFFFFFFF, 0xFF404040, 0xFF808080, 0xFFC0C0C0, 0x00000000
};

static inline float mo_clampf(float a, float lo, float hi)
{
    return (a > hi) ? hi : ((a < lo) ? lo : a);
}

#ifdef MO_WIN32
static LARGE_INTEGER g_moTimerFrequency = {{0}};
void mo_timer_init(mo_timer* pTimer)
{
	if (g_moTimerFrequency.QuadPart == 0) {
        QueryPerformanceFrequency(&g_moTimerFrequency);
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    pTimer->counter = (uint64_t)counter.QuadPart;
}

double mo_timer_tick(mo_timer* pTimer)
{
	LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter)) {
        return 0;
    }

    long long newTimeCounter = counter.QuadPart;
    long long oldTimeCounter = pTimer->counter;
    
    pTimer->counter = newTimeCounter;

    return (newTimeCounter - oldTimeCounter) / (double)g_moTimerFrequency.QuadPart;
}
#endif

#ifdef MO_X11
void mo_timer_init(mo_timer* pTimer)
{
	struct timespec newTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &newTime);
	
    pTimer->counter = (newTime.tv_sec * 1000000000LL) + newTime.tv_nsec;
}

double mo_timer_tick(mo_timer* pTimer)
{
	struct timespec newTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &newTime);

    long long newTimeCounter = (newTime.tv_sec * 1000000000LL) + newTime.tv_nsec;
    long long oldTimeCounter = pTimer->counter;

    pTimer->counter = newTimeCounter;

    return (newTimeCounter - oldTimeCounter) / 1000000000.0;
}
#endif





static inline void mo__on_button_down(mo_context* pContext, unsigned int button)
{
    if (button == 0) return;
    if ((pContext->buttonState & button) == 0) {
		pContext->buttonState |= button;
		pContext->buttonPressState |= button;
		pContext->buttonReleaseState &= ~button;
	}
}

static inline void mo__on_button_up(mo_context* pContext, unsigned int button)
{
    if (button == 0) return;
    pContext->buttonState &= ~button;
	pContext->buttonPressState &= ~button;
	pContext->buttonReleaseState |= button;
}

#ifdef _WIN32
static const char* g_MintaroWndClassName = "mintaro.WindowClass";
static LONG g_MintaroInitCounter = 0;

static mo_bool32 mo_is_win32_mouse_button_key_code(WPARAM wParam)
{
    return wParam == VK_LBUTTON || wParam == VK_RBUTTON || wParam == VK_MBUTTON || wParam == VK_XBUTTON1 || wParam == VK_XBUTTON2;
}

static mo_key mo_convert_key_code__win32(WPARAM wParam)
{
    switch (wParam)
    {
        case VK_BACK:   return MO_KEY_BACKSPACE;
        case VK_RETURN: return MO_KEY_ENTER;
        case VK_SHIFT:  return MO_KEY_SHIFT;
        case VK_ESCAPE: return MO_KEY_ESCAPE;
        case VK_SPACE:  return MO_KEY_SPACE;
        case VK_PRIOR:  return MO_KEY_PAGE_UP;
        case VK_NEXT:   return MO_KEY_PAGE_DOWN;
        case VK_END:    return MO_KEY_END;
        case VK_HOME:   return MO_KEY_HOME;
        case VK_LEFT:   return MO_KEY_ARROW_LEFT;
        case VK_UP:     return MO_KEY_ARROW_UP;
        case VK_RIGHT:  return MO_KEY_ARROW_RIGHT;
        case VK_DOWN:   return MO_KEY_ARROW_DOWN;
        case VK_DELETE: return MO_KEY_DELETE;
        case VK_F1:     return MO_KEY_F1;
        case VK_F2:     return MO_KEY_F2;
        case VK_F3:     return MO_KEY_F3;
        case VK_F4:     return MO_KEY_F4;
        case VK_F5:     return MO_KEY_F5;
        case VK_F6:     return MO_KEY_F6;
        case VK_F7:     return MO_KEY_F7;
        case VK_F8:     return MO_KEY_F8;
        case VK_F9:     return MO_KEY_F9;
        case VK_F10:    return MO_KEY_F10;
        case VK_F11:    return MO_KEY_F11;
        case VK_F12:    return MO_KEY_F12;
        default: break;
    }

    return (mo_key)wParam;
}

static LRESULT DefaultWindowProcWin32(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    mo_context* pContext = (mo_context*)GetWindowLongPtrA(hWnd, 0);
    if (pContext == NULL) {
        return DefWindowProcA(hWnd, msg, wParam, lParam);
    }

    switch (msg)
    {
        case WM_CLOSE:
        {
            mo_close(pContext);
            return 0;
        }

        case WM_SIZE:
        {
            pContext->windowWidth = LOWORD(lParam);
            pContext->windowHeight = HIWORD(lParam);
        } break;

        case WM_KEYDOWN:
        {
            if (!mo_is_win32_mouse_button_key_code(wParam)) {
                if ((lParam & (1 << 30)) == 0) {    // <-- This checks for auto-repeat. We want to ignore auto-repeated key-down events.
                    mo__on_button_down(pContext, mo_get_key_binding(pContext, mo_convert_key_code__win32(wParam)));
                }
            }
        } break;

        case WM_KEYUP:
        {
            mo__on_button_up(pContext, mo_get_key_binding(pContext, mo_convert_key_code__win32(wParam)));
        } break;

        default: break;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}
#else
static int g_MintaroInitCounter = 0;
static Display* g_moX11Display = NULL;
static Atom g_WM_DELETE_WINDOW = 0;
static Atom g_Atom_moWindow = 0;

static inline void mo_set_x11_window_property(Display* display, Window window, Atom property, const void* pUserData)
{
    XChangeProperty(display, window, property, XA_INTEGER, 8, PropModeReplace, (const unsigned char*)&pUserData, sizeof(pUserData));
}

static inline void* mo_get_x11_window_property(Display* display, Window window, Atom property)
{
    Atom actualType;
    int unused1;
    unsigned long unused2;
    unsigned long unused3;

    unsigned char* pRawData;
    XGetWindowProperty(display, window, property, 0, sizeof(void*), False, XA_INTEGER, &actualType, &unused1, &unused2, &unused3, &pRawData);

    void* pUserData;
    mo_copy_memory(&pUserData, pRawData, sizeof(pUserData));

    XFree(pRawData);
    return pUserData;
}

void mo_x11_create_presentation_buffer(mo_context* pContext, unsigned int sizeX, unsigned int sizeY)
{
	if (pContext == NULL || pContext->pPresentBufferX11 != NULL) return;
	if (sizeX == 0) sizeX = 1;
	if (sizeY == 0) sizeY = 1;
	
	unsigned int depth = 24;	// Setting this to 32 doesn't work in my testing.

	if (pContext->flags & MO_FLAG_X11_USING_SHM) {
		XImage* pImage = XShmCreateImage(g_moX11Display, CopyFromParent, depth, ZPixmap, 0,
			&pContext->shmInfo, sizeX, sizeY);
		if (pImage == NULL) {
			return;	// Failed to create the image.
		}
		
		pContext->shmInfo.shmid = shmget(IPC_PRIVATE, pImage->bytes_per_line * pImage->height, IPC_CREAT | 0777);
		if (pContext->shmInfo.shmid < 0) {
			XDestroyImage(pImage);
			return;	// Failed to allocate shared memory.
		}
		
		pContext->shmInfo.shmaddr = pImage->data = (char*)shmat(pContext->shmInfo.shmid, 0, 0);
		if (pContext->shmInfo.shmaddr == (char*)-1) {
			XDestroyImage(pImage);
			return;
		}

		pContext->shmInfo.readOnly = False;
		XShmAttach(g_moX11Display, &pContext->shmInfo);
		
		pContext->pPresentBufferX11 = pImage;
	} else {
		char* pImageData = (char*)mo_malloc(sizeX * sizeY * 4);
		if (pImageData == NULL) {
			return;	// Out of memory.
		}
		
		pContext->pPresentBufferX11 = XCreateImage(g_moX11Display, CopyFromParent, depth, ZPixmap, 0,
			pImageData, sizeX, sizeY, 32, 0);
		if (pContext->pPresentBufferX11 == NULL) {
			return;	// Failed to create the image.
		}
	}
}

void mo_x11_delete_presentation_buffer(mo_context* pContext)
{
	if (pContext == NULL || pContext->pPresentBufferX11 == NULL) return;
	
	if (pContext->flags & MO_FLAG_X11_USING_SHM) {
		XShmDetach(g_moX11Display, &pContext->shmInfo);
		XDestroyImage(pContext->pPresentBufferX11);
		shmdt(pContext->shmInfo.shmaddr);
	} else {
		mo_free(pContext->pPresentBufferX11->data);
		XDestroyImage(pContext->pPresentBufferX11);
	}
	
	pContext->pPresentBufferX11 = NULL;
}

void mo_x11_resize_presentation_buffer(mo_context* pContext, unsigned int sizeX, unsigned int sizeY)
{
	if (pContext == NULL) return;
	mo_x11_delete_presentation_buffer(pContext);
	mo_x11_create_presentation_buffer(pContext, sizeX, sizeY);
}

void mo_x11_present(mo_context* pContext)
{
	if (pContext == NULL || pContext->pPresentBufferX11 == NULL) return;
	
	if (pContext->flags & MO_FLAG_X11_USING_SHM) {
		XShmPutImage(g_moX11Display, pContext->windowX11, pContext->gcX11, pContext->pPresentBufferX11,
			0, 0, 0, 0, pContext->pPresentBufferX11->width, pContext->pPresentBufferX11->height, False);
	} else {
		XPutImage(g_moX11Display, pContext->windowX11, pContext->gcX11, pContext->pPresentBufferX11,
			0, 0, 0, 0, pContext->pPresentBufferX11->width, pContext->pPresentBufferX11->height);
	}
}
#endif



///////////////////////////////////////////////////////////////////////////////
//
// AUDIO
//
///////////////////////////////////////////////////////////////////////////////
void mo_on_log__mal(mal_device* pDevice, const char* message)
{
    mo_logf((mo_context*)pDevice->pUserData, "[AUDIO] %s", message);
}

mo_uint32 mo_sound__read_and_accumulate_frames(mo_sound* pSound, float linearVolume, mo_uint32 frameCount, mo_int16* pFrames)
{
    // This is the main mixing function. pFrames is an in/out buffer - samples are read from the sound's data source
    // and then accumated with the samples already in the buffer.
    //
    // When a sound reaches the end of it's data source it will either loop or just stop. If it's an inline sound it
    // will be _marked_ for deletion (actual deletion will happen later at the end of the next step). The reason it
    // it only marked for deletion is because Mintaro is not thread-safe and this function will be called on another
    // thread to the main stepping thread.

    // TODO: Add support for FLAC and Vorbis.

    // Currently assuming the device is stereo. When/if different channel counts are supported we'll need to look
    // into making this more robust.
    mo_assert(pSound->pContext->playbackDevice2.channels == 2);

    const mo_uint32 soundChannels = pSound->pSource->channels;
    const mo_uint32 deviceChannels = pSound->pContext->playbackDevice2.channels;

    mo_uint32 totalFramesRead = 0;
    while (frameCount > 0) {
        mo_uint64 framesAvailable = (pSound->pSource->sampleCount - pSound->currentSample) / pSound->pSource->channels;
        if (framesAvailable > frameCount) {
            framesAvailable = frameCount;
        }

        if (soundChannels == 1) {
            // Mono
            for (mo_uint32 iFrame = 0; iFrame < framesAvailable; ++iFrame) {
                float scaledSample0 = pSound->pSource->pSampleData[pSound->currentSample + iFrame] * linearVolume;
                float outputSample0 = pFrames[iFrame*deviceChannels + 0] + scaledSample0;
                pFrames[iFrame*deviceChannels + 0] = (mo_int16)(mo_clampf(outputSample0, -32768.0f, 32767.0f));
                pFrames[iFrame*deviceChannels + 1] = (mo_int16)(mo_clampf(outputSample0, -32768.0f, 32767.0f));
            }
            pSound->currentSample += framesAvailable * soundChannels;
        } else if (soundChannels == 2) {
            // Stereo
            for (mo_uint32 iFrame = 0; iFrame < framesAvailable; ++iFrame) {
                float scaledSample0 = pSound->pSource->pSampleData[pSound->currentSample + iFrame*soundChannels + 0] * linearVolume;
                float scaledSample1 = pSound->pSource->pSampleData[pSound->currentSample + iFrame*soundChannels + 1] * linearVolume;
                float outputSample0 = pFrames[iFrame*deviceChannels + 0] + scaledSample0;
                float outputSample1 = pFrames[iFrame*deviceChannels + 1] + scaledSample1;
                pFrames[iFrame*deviceChannels + 0] = (mo_int16)(mo_clampf(outputSample0, -32768.0f, 32767.0f));
                pFrames[iFrame*deviceChannels + 1] = (mo_int16)(mo_clampf(outputSample1, -32768.0f, 32767.0f));
            }
            pSound->currentSample += framesAvailable * soundChannels;
        } else {
            // More than stereo. Just drop the extra channels. This can be used for stereo sounds, but is not as optimized.
            for (mo_uint32 iFrame = 0; iFrame < framesAvailable; ++iFrame) {
                for (mo_uint32 iChannel = 0; iChannel < deviceChannels; ++iChannel) {
                    float scaledSample0 = pSound->pSource->pSampleData[pSound->currentSample + iFrame*soundChannels + iChannel] * linearVolume;
                    float outputSample0 = pFrames[iFrame*deviceChannels + iChannel] + scaledSample0;
                    pFrames[iFrame*deviceChannels + iChannel] = (mo_int16)(mo_clampf(outputSample0, -32768.0f, 32767.0f));
                }
            }

            pSound->currentSample += framesAvailable * soundChannels;
        }


        mo_bool32 reachedEnd = framesAvailable < frameCount;
        frameCount -= (mo_uint32)framesAvailable;   // <-- Safe cast because we clamped it to frameCount which is 32-bit.
        pFrames += framesAvailable * deviceChannels;

        if (reachedEnd) {
            if (mo_sound_is_looping(pSound)) {
                pSound->currentSample = 0;
            } else {
                if ((pSound->flags & MO_SOUND_FLAG_INLINED) != 0) {
                    mo_sound_mark_for_deletion(pSound);
                } else {
                    mo_sound_stop(pSound);
                }

                break;
            }
        }
    }

    return totalFramesRead;
}

mal_uint32 mo_on_send_frames__mal(mal_device* pDevice, mal_uint32 frameCount, void* pFrames)
{
    // This is where all of our audio mixing is done.
    mo_context* pContext = (mo_context*)pDevice->pUserData;
    mo_assert(pContext != NULL);

    // The output buffer is in s16 format.
    mo_int16* pFramesS16 = (mal_int16*)pFrames;
    mo_assert(pFramesS16 != NULL);

    // Mixing is easy - we just need to accumulate each sound, making sure we adjust for volume. If a sound reaches
    // the end of it's data source we need to either loop or stop the sound.

    // Important that we clear the output buffer to zero since we'll be accumulating.
    mo_zero_memory(pFrames, frameCount * pDevice->channels * sizeof(mo_int16));

    for (mo_uint32 iSound = 0; iSound < pContext->soundCount; ++iSound) {
        mo_sound* pSound = pContext->ppSounds[iSound];
        if (mo_sound_is_playing(pSound) && !mo_sound_group_is_paused(pContext, pSound->group)) {
            float linearVolume = pSound->linearVolume * pContext->soundGroups[pSound->group].linearVolume * pContext->soundGroups[MO_SOUND_GROUP_MASTER].linearVolume;
            if (linearVolume > 0) {
                mo_sound__read_and_accumulate_frames(pSound, linearVolume, frameCount, pFramesS16);
            }
        }
    }

    return frameCount;
}

mo_result mo_init_audio(mo_context* pContext)
{
    mo_assert(pContext != NULL);

    // Sound groups.
    for (int i = 0; i < MO_SOUND_GROUP_COUNT; ++i) {
        pContext->soundGroups[i].linearVolume = 1;
    }
    
    mal_device_config config;
    config.format = mal_format_s16;
    config.channels = 2;
    config.sampleRate = 44100;
    config.bufferSizeInFrames = 0;
    config.periods = 0;
    config.onSendCallback = mo_on_send_frames__mal;
    config.onRecvCallback = NULL;
    config.onStopCallback = NULL;
    config.onLogCallback  = mo_on_log__mal;
    mal_result resultMAL = mal_device_init(&pContext->playbackDevice2, mal_device_type_playback, NULL, &config, pContext);
    if (resultMAL != MAL_SUCCESS) {
        return MO_ERROR;
    }

    // Start the device now, but we might want to make this a bit more intelligent and only have the
    // device running while a sound needs to be played.
    mal_device_start(&pContext->playbackDevice2);

    return MO_SUCCESS;
}

void mo_uninit_audio(mo_context* pContext)
{
    mo_assert(pContext != NULL);

    mal_device_uninit(&pContext->playbackDevice2);
}


mo_color_rgba mo_make_rgba(mo_uint8 r, mo_uint8 g, mo_uint8 b, mo_uint8 a)
{
	mo_color_rgba result;
	result.b = b;
	result.g = g;
	result.r = r;
	result.a = a;
	return result;
}

mo_color_rgba mo_make_rgb(mo_uint8 r, mo_uint8 g, mo_uint8 b)
{
	return mo_make_rgba(r, g, b, 255);
}

mo_result mo_init(mo_profile* pProfile, mo_uint32 windowSizeX, mo_uint32 windowSizeY, const char* title, mo_on_step_proc onStep, void* pUserData, mo_context** ppContext)
{
    if (ppContext == NULL) return MO_INVALID_ARGS;
    mo_zero_object(ppContext);

    mo_profile defaultProfile;
    defaultProfile.resolutionX = 160;
    defaultProfile.resolutionY = 144;
	defaultProfile.transparentColorIndex = 255;
    defaultProfile.paletteSize = 256;
    mo_copy_memory(defaultProfile.palette, g_moDefaultPalette, 256*4);
    if (pProfile == NULL) pProfile = &defaultProfile;
    if (pProfile->paletteSize == 0) return MO_BAD_PROFILE;
    if (pProfile->transparentColorIndex >= pProfile->paletteSize) return MO_BAD_PROFILE;

    if (windowSizeX == 0) windowSizeX = pProfile->resolutionX;
    if (windowSizeY == 0) windowSizeY = pProfile->resolutionY;
    if (title == NULL) title = "Mintaro";

    size_t screenSizeInBytes = pProfile->resolutionX * pProfile->resolutionY * sizeof(mo_color_index);
    size_t contextSize = sizeof(mo_context) + screenSizeInBytes;

    mo_context* pContext = (mo_context*)mo_calloc(contextSize);
    if (pContext == NULL) {
        return MO_OUT_OF_MEMORY;
    }

	pContext->onStep = onStep;
	pContext->pUserData = pUserData;
    pContext->profile = *pProfile;
    pContext->screen = pContext->pExtraData;
	
	// The window.
#ifdef MO_WIN32
    if (mo_atomic_increment(&g_MintaroInitCounter) == 1) {
        WNDCLASSEXA wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize        = sizeof(wc);
        wc.cbWndExtra    = sizeof(void*);
        wc.lpfnWndProc   = (WNDPROC)DefaultWindowProcWin32;
        wc.lpszClassName = g_MintaroWndClassName;
        wc.hCursor       = LoadCursorA(NULL, MAKEINTRESOURCEA(32512));
        wc.style         = CS_OWNDC | CS_DBLCLKS;
        if (!RegisterClassExA(&wc)) {
            mo_free(pContext);
            return MO_FAILED_TO_INIT_PLATFORM;   // Failed to initialize the window class.
        }
    }


    DWORD dwExStyle = 0;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    pContext->hWnd = CreateWindowExA(dwExStyle, g_MintaroWndClassName, title, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowSizeX, windowSizeY, NULL, NULL, NULL, NULL);
    if (pContext->hWnd == NULL) {
        mo_uninit(pContext);
        return MO_FAILED_TO_INIT_PLATFORM;
    }

    // Cache the DC so we can avoid GetDC().
    pContext->hDC = GetDC(pContext->hWnd);

    // The context needs to be linked to the Win32 window handle so it can be accessed from the event handler.
    SetWindowLongPtrA(pContext->hWnd, 0, (LONG_PTR)pContext);

    // We should have a window, but before showing it we need to make a few small adjustments to the size such that the client size
    // is equal to resolutionX and resolutionY. When we created the window, we specified resolutionX and resolutionY as the dimensions,
    // however this includes the size of the outer border. The outer border should not be included, so we need to stretch the window
    // just a little bit such that the area inside the borders are exactly equal to resolutionX and resolutionY.
    RECT windowRect;
    RECT clientRect;
    GetWindowRect(pContext->hWnd, &windowRect);
    GetClientRect(pContext->hWnd, &clientRect);

    pContext->windowWidth  = (int)windowSizeX + ((windowRect.right - windowRect.left) - (clientRect.right - clientRect.left));
    pContext->windowHeight = (int)windowSizeY + ((windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top));
    SetWindowPos(pContext->hWnd, NULL, 0, 0, pContext->windowWidth, pContext->windowHeight, SWP_NOZORDER | SWP_NOMOVE);


    // We need a DIB section for drawing the screen to the Win32 window.
    pContext->hDIBDC = CreateCompatibleDC(pContext->hDC);
    if (pContext->hDIBDC == NULL) {
        mo_uninit(pContext);
        return MO_FAILED_TO_INIT_PLATFORM;
    }

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = pProfile->resolutionX;
    bmi.bmiHeader.biHeight = -(int)pProfile->resolutionY;     // <-- Making this negative makes it top-down.
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    pContext->hDIBSection = CreateDIBSection(pContext->hDIBDC, &bmi, DIB_RGB_COLORS, &pContext->pScreenRGBA_DIB, NULL, 0);
    if (pContext->hDIBSection == NULL) {
        mo_uninit(pContext);
        return MO_FAILED_TO_INIT_PLATFORM;
    }

    SelectObject(pContext->hDIBDC, pContext->hDIBSection);

    ShowWindow(pContext->hWnd, SW_SHOWNORMAL);
#endif

#ifdef MO_X11
    if (mo_atomic_increment(&g_MintaroInitCounter) == 1) {
        //assert(g_moX11Display == NULL);
		g_moX11Display = XOpenDisplay(NULL);
		if (g_moX11Display == NULL) {
            mo_free(pContext);
			return MO_FAILED_TO_INIT_PLATFORM;
		}
		
		g_WM_DELETE_WINDOW = XInternAtom(g_moX11Display, "WM_DELETE_WINDOW", False);
    	g_Atom_moWindow = XInternAtom(g_moX11Display, "ATOM_moWindow", False);
    }
	
	int blackColor = BlackPixel(g_moX11Display, DefaultScreen(g_moX11Display));
    int whiteColor = WhitePixel(g_moX11Display, DefaultScreen(g_moX11Display));
	
	pContext->windowX11 = XCreateSimpleWindow(g_moX11Display, DefaultRootWindow(g_moX11Display),
		0, 0, windowSizeX, windowSizeY, 0, blackColor, blackColor);
	if (pContext->windowX11 == 0) {
		mo_uninit(pContext);
		return MO_FAILED_TO_INIT_PLATFORM;
	}
	
	mo_set_x11_window_property(g_moX11Display, pContext->windowX11, g_Atom_moWindow, pContext);
	
	XSelectInput(g_moX11Display, pContext->windowX11, ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask);
    XSetWMProtocols(g_moX11Display, pContext->windowX11, &g_WM_DELETE_WINDOW, 1);
    XStoreName(g_moX11Display, pContext->windowX11, title);
	XMapRaised(g_moX11Display, pContext->windowX11);
	
	pContext->gcX11 = XCreateGC(g_moX11Display, pContext->windowX11, 0, NULL);
	if (pContext->gcX11 == 0) {
		mo_uninit(pContext);
		return MO_FAILED_TO_INIT_PLATFORM;
	}
	
	// By default every KeyPress event is matched with a KeyRelease event even when the use hasn't
	// actually released the key. This call makes it so the KeyRelease event is only posted when the
	// user has actually released the key.
	XkbSetDetectableAutoRepeat(g_moX11Display, True, NULL);
	
	// We want to use the MIT-SHM extension if it's available.
	if (XShmQueryExtension(g_moX11Display)) {
		pContext->flags |= MO_FLAG_X11_USING_SHM;
	}
#endif

    // Audio.
    mo_result result = mo_init_audio(pContext);
    if (result != MO_SUCCESS) {
        mo_uninit(pContext);
        return result;
    }


    // Default key bindings.
    mo_bind_key_to_button(pContext, MO_KEY_ARROW_LEFT, MO_BUTTON_LEFT);
    mo_bind_key_to_button(pContext, MO_KEY_ARROW_UP, MO_BUTTON_UP);
    mo_bind_key_to_button(pContext, MO_KEY_ARROW_RIGHT, MO_BUTTON_RIGHT);
    mo_bind_key_to_button(pContext, MO_KEY_ARROW_DOWN, MO_BUTTON_DOWN);
    mo_bind_key_to_button(pContext, 'Z', MO_BUTTON_A);
    mo_bind_key_to_button(pContext, 'X', MO_BUTTON_B);
    mo_bind_key_to_button(pContext, MO_KEY_SPACE, MO_BUTTON_SELECT);
    mo_bind_key_to_button(pContext, MO_KEY_ENTER, MO_BUTTON_START);

	// Timer.
	mo_timer_init(&pContext->timer);

    *ppContext = pContext;
	return MO_SUCCESS;
}

void mo_uninit(mo_context* pContext)
{
	if (pContext == NULL) return;

    mo_uninit_audio(pContext);

#ifdef MO_WIN32
    if (pContext->hDIBSection) {
        DeleteObject(pContext->hDIBSection);
    }
    if (pContext->hDIBDC) {
        DeleteDC(pContext->hDIBDC);
    }
    if (pContext->hWnd) {
        DestroyWindow(pContext->hWnd);
    }
    if (mo_atomic_decrement(&g_MintaroInitCounter) == 0) {
        UnregisterClassA(g_MintaroWndClassName, GetModuleHandleA(NULL));
    }
#endif

#ifdef MO_X11
    if (pContext->gcX11) {
        XFreeGC(g_moX11Display, pContext->gcX11);
    }
	if (pContext->windowX11) {
		XDestroyWindow(g_moX11Display, pContext->windowX11);
	}
    if (mo_atomic_decrement(&g_MintaroInitCounter) == 0) {
	    XCloseDisplay(g_moX11Display);
    }
#endif

    mo_free(pContext);
}

void mo_present(mo_context* pContext)
{
	if (pContext == NULL) return;
    
#ifdef MO_WIN32
    // Conveniently, we can get Win32 to do the scaling for us. This means we're able to do an efficient 1x1 copy ourselves and then
    // let the OS do the rest for us. Of course, there's a chance we could do it more efficiently ourselves, but maybe not.

    // OPTIMIZATION NOTES
    // ==================
    // - It's more efficient to flip the screen ourselves than passing in a negative height to StretchDIBits().
    // - CreateDIBSection() + StretchBlt() is more efficient than StretchDIBits() by a tiny amount.
    // - BitBlt() is a tiny bit faster than StretchBlt(), but it's less than a microsecond. Probably not worth it.
	//
	// THINGS TO TRY
	// -------------
	// [DONE] Try using a palette (DIB_PAL_COLORS).
	//     RESULT: Couldn't figure out how to get it working properly. Probably not worthwhile.

#if 0
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = screenWidth;
    bmi.bmiHeader.biHeight = screenHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    for (int y = 0; y < screenHeight; ++y) {
		mo_uint32* pDstRow = ((mo_uint32*)pContext->pScreenRGBA) + (y * screenWidth);
		for (int x = 0; x < screenWidth; ++x) {
			unsigned int screenX = x;
			unsigned int screenY = y;
            //pDstRow[x] = pContext->palette[(pContext->screen[screenY][screenX] & 0x03)].rgba;
            pDstRow[x] = pContext->palette[pContext->screen[screenHeight - screenY - 1][screenX]].rgba;    // <-- Needs to be upside down for Win32. Can also use a negative scale in StretchDIBits()
		}
	}

    // Note how the height and Y position is flipped in order to make it the right way around.
    //
    // Questions:
    //   - Does flipping upside down force it down a slower path? A) Yes, it's slower.
    //StretchDIBits(pContext->hDC, 0, pContext->windowHeight-1, pContext->windowWidth, -pContext->windowHeight, 0, 0, screenWidth, screenHeight, pContext->pScreenRGBA, &bmi, DIB_RGB_COLORS, SRCCOPY);
    StretchDIBits(pContext->hDC, 0, 0, pContext->windowWidth, pContext->windowHeight, 0, 0, screenWidth, screenHeight, pContext->pScreenRGBA, &bmi, DIB_RGB_COLORS, SRCCOPY);
#else
    // Before writing the data to the DIB section we need to flush GDI.
    //GdiFlush();

    for (unsigned int y = 0; y < pContext->profile.resolutionY; ++y) {
		mo_uint32* pDstRow = ((mo_uint32*)pContext->pScreenRGBA_DIB) + (y * pContext->profile.resolutionX);
		for (unsigned int x = 0; x < pContext->profile.resolutionX; ++x) {
			unsigned int screenX = x;
			unsigned int screenY = y;
            pDstRow[x] = pContext->profile.palette[pContext->screen[screenY*pContext->profile.resolutionX + screenX]].rgba;
            //pDstRow[x] = pContext->palette[pContext->screen[screenHeight - screenY - 1][screenX]].rgba;    // <-- Needs to be upside down for Win32. Can also use a negative scale in StretchDIBits()
		}
	}

    //BitBlt(pContext->hDC, 0, 0, pContext->windowWidth, pContext->windowHeight, pContext->hDIBDC, 0, 0, SRCCOPY);
    StretchBlt(pContext->hDC, 0, 0, pContext->windowWidth, pContext->windowHeight, pContext->hDIBDC, 0, 0, pContext->profile.resolutionX, pContext->profile.resolutionY, SRCCOPY);
#endif
#endif

#ifdef MO_X11
	// OPTIMZATION NOTES
	// =================
	// - The MIT-SHM extension using XShmPutImage() is about 7 microseconds faster than XPutImage() @ 160x144.
	//
	//
	// THINGS TO TRY
	// -------------
	// [DONE] MIT-SHM Extension: https://linux.die.net/man/3/xshmputimage
	//     RESULT: A good optimization. About 7 microseconds faster @ 160x144 and scales with higher resolutions.
	
	if (pContext->pPresentBufferX11 == NULL) return;

	unsigned int dstSizeX = pContext->pPresentBufferX11->width;
	unsigned int dstSizeY = pContext->pPresentBufferX11->height;
	unsigned int srcSizeX = pContext->profile.resolutionX;
	unsigned int srcSizeY = pContext->profile.resolutionY;
	
	float ratioX = (float)srcSizeX / dstSizeX;
	float ratioY = (float)srcSizeY / dstSizeY;
	
	for (int y = 0; y < dstSizeY; ++y) {
		mo_uint32* pDstRow = ((mo_uint32*)pContext->pPresentBufferX11->data) + (y * dstSizeX);
		for (int x = 0; x < dstSizeX; ++x) {
			unsigned int screenX = (unsigned int)(x*ratioX);
			unsigned int screenY = (unsigned int)(y*ratioY);
		
			pDstRow[x] = pContext->profile.palette[pContext->screen[screenY*pContext->profile.resolutionX + screenX]].rgba;
		}
	}
	
	mo_x11_present(pContext);
#endif
}

#ifdef MO_X11
static mo_key mo_convert_key_code__x11(unsigned int keycode)
{
    KeySym key = XkbKeycodeToKeysym(g_moX11Display, keycode, 0, 1);
    switch (key)
    {
        case XK_BackSpace: return MO_KEY_BACKSPACE;
        case XK_Return:    return MO_KEY_ENTER;
        case XK_Shift_L:   return MO_KEY_SHIFT;
        case XK_Shift_R:   return MO_KEY_SHIFT;
        case XK_Escape:    return MO_KEY_ESCAPE;
        case XK_space:     return MO_KEY_SPACE;
        case XK_Page_Up:   return MO_KEY_PAGE_UP;
        case XK_Page_Down: return MO_KEY_PAGE_DOWN;
        case XK_End:       return MO_KEY_END;
        case XK_Home:      return MO_KEY_HOME;
        case XK_Left:      return MO_KEY_ARROW_LEFT;
        case XK_Up:        return MO_KEY_ARROW_UP;
        case XK_Right:     return MO_KEY_ARROW_RIGHT;
        case XK_Down:      return MO_KEY_ARROW_DOWN;
        case XK_Delete:    return MO_KEY_DELETE;
        case XK_F1:        return MO_KEY_F1;
        case XK_F2:        return MO_KEY_F2;
        case XK_F3:        return MO_KEY_F3;
        case XK_F4:        return MO_KEY_F4;
        case XK_F5:        return MO_KEY_F5;
        case XK_F6:        return MO_KEY_F6;
        case XK_F7:        return MO_KEY_F7;
        case XK_F8:        return MO_KEY_F8;
        case XK_F9:        return MO_KEY_F9;
        case XK_F10:       return MO_KEY_F10;
        case XK_F11:       return MO_KEY_F11;
        case XK_F12:       return MO_KEY_F12;
        default: break;
    }

    return (mo_key)key;
}

void mo_handle_x11_event(XEvent* ex)
{
	mo_context* pContext = (mo_context*)mo_get_x11_window_property(g_moX11Display, ex->xany.window, g_Atom_moWindow);
	if (pContext == NULL) {
		return;
	}

    switch (ex->type)
    {
        case ConfigureNotify:
        {
			if (pContext->pPresentBufferX11 == NULL || (pContext->pPresentBufferX11->width != ex->xconfigure.width || pContext->pPresentBufferX11->height != ex->xconfigure.height)) {
				mo_x11_resize_presentation_buffer(pContext, ex->xconfigure.width, ex->xconfigure.height);
			}
        } break;


        case MotionNotify:
        {
        } break;

        case ButtonPress:
        {
        } break;

        case ButtonRelease:
        {
        } break;


        case KeyPress:
        {
			mo__on_button_down(pContext, mo_get_key_binding(pContext, mo_convert_key_code__x11(ex->xkey.keycode)));
        } break;

        case KeyRelease:
        {
			mo__on_button_up(pContext, mo_get_key_binding(pContext, mo_convert_key_code__x11(ex->xkey.keycode)));
        } break;
		
		
		case Expose:
		{
			// We need to present the screen to the window. To do this we need to write the
			// data to an XImage object and then call XPutImage() to copy the image over to
			// the window.
			mo_present(pContext);
			XFlush(g_moX11Display);	// <-- Is this needed? Assuming so because I saw it in an example, but not sure.
		} break;


        default: break;
    }
}
#endif

int mo_run(mo_context* pContext)
{
	if (pContext == NULL) return MO_INVALID_ARGS;
	
	while ((pContext->flags & MO_FLAG_CLOSING) == 0) {
		// Handle window events first.
#ifdef MO_WIN32
        MSG msg;
        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) {
                return (int)msg.wParam;  // Received a quit message.
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
#endif

#ifdef MO_X11
		if (XPending(g_moX11Display) > 0) {   // <-- Use a while loop instead?
            XEvent x11Event;
            XNextEvent(g_moX11Display, &x11Event);

            if (x11Event.type == ClientMessage) {
                if ((Atom)x11Event.xclient.data.l[0] == g_WM_DELETE_WINDOW) {
                    return 0;   // Received a quit message.
                }
            };

			mo_handle_x11_event(&x11Event);
        }
#endif

		// Now just step the game.
		double dt = mo_timer_tick(&pContext->timer);
		if (pContext->onStep) {
			pContext->onStep(pContext, dt);
			
			pContext->buttonPressState = 0;
			pContext->buttonReleaseState = 0;
		}
		
		// Present the screen to the window.
		mo_present(pContext);
	}

    return 0;
}

void mo_close(mo_context* pContext)
{
    if (pContext == NULL) return;

#ifdef MO_WIN32
	PostQuitMessage(0);
#endif

	pContext->flags |= MO_FLAG_CLOSING;
}

void mo_log(mo_context* pContext, const char* message)
{
    if (pContext == NULL || pContext->onLog == NULL) return;
    pContext->onLog(pContext, message);
}

void mo_logf(mo_context* pContext, const char* format, ...)
{
    va_list args;
	
    va_start(args, format);
#if defined(_MSC_VER)
    int len = _vscprintf(format, args);
#else
    int len = vsnprintf(NULL, 0, format, args);
#endif
	va_end(args);
	
    if (len < 0) {
        return;
    }

    char* message = (char*)mo_malloc(len+1);
    if (message == NULL) {
        va_end(args);
        return;
    }

	va_start(args, format);
#if defined(_MSC_VER)
    len = vsprintf_s(message, len+1, format, args);
#else
    len = vsnprintf(message, len+1, format, args);
#endif
	va_end(args);

    mo_log(pContext, message);

    mo_free(message);
    va_end(args);
}


//// Resources ////

static const char* mo_file_name(const char* path)
{
    if (path == NULL) {
        return NULL;
    }

    const char* fileName = path;

    // We just loop through the path until we find the last slash.
    while (path[0] != '\0') {
        if (path[0] == '/' || path[0] == '\\') {
            fileName = path;
        }

        path += 1;
    }

    // At this point the file name is sitting on a slash, so just move forward.
    while (fileName[0] != '\0' && (fileName[0] == '/' || fileName[0] == '\\')) {
        fileName += 1;
    }

    return fileName;
}

static const char* mo_extension(const char* path)
{
    if (path == NULL) {
        return NULL;
    }

    const char* extension     = mo_file_name(path);
    const char* lastoccurance = 0;

    // Just find the last '.' and return.
    while (extension[0] != '\0')
    {
        if (extension[0] == '.') {
            extension    += 1;
            lastoccurance = extension;
        }

        extension += 1;
    }

    return (lastoccurance != 0) ? lastoccurance : extension;
}

static mo_bool32 mo_extension_equal(const char* path, const char* extension)
{
    if (path == NULL || extension == NULL) {
        return MO_FALSE;
    }

    const char* ext1 = extension;
    const char* ext2 = mo_extension(path);

#ifdef _MSC_VER
    return _stricmp(ext1, ext2) == 0;
#else
    return strcasecmp(ext1, ext2) == 0;
#endif
}

static void* mo_open_and_read_file_with_extra_data(mo_context* pContext, const char* filePath, size_t* pFileSizeOut, size_t extraBytes)
{
    if (pFileSizeOut) *pFileSizeOut = 0;   // For safety.

    if (filePath == NULL) {
        return NULL;
    }
	
#ifdef MO_WIN32
    HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        mo_logf(pContext, "Could not find file: %s", filePath);
        return NULL;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize) || ((fileSize.QuadPart + extraBytes) > 0xFFFFFFFF)) {
        mo_logf(pContext, "File is too large: %s", filePath);
        CloseHandle(hFile);
        return NULL;
    }

    void* pFileData = mo_malloc(fileSize.LowPart + extraBytes);
	if (pFileData == NULL) {
		mo_logf(pContext, "Not enough memory to load image: %s", filePath);
		CloseHandle(hFile);
		return NULL;
	}

    DWORD bytesRead;
    if (!ReadFile(hFile, pFileData, fileSize.LowPart, &bytesRead, NULL) || bytesRead != fileSize.LowPart) {
        mo_logf(pContext, "Failed to read image file: %s", filePath);
		mo_free(pFileData);
        CloseHandle(hFile);
        return NULL;
    }
	
	CloseHandle(hFile);
	
	if (pFileSizeOut) *pFileSizeOut = (unsigned int)fileSize.LowPart;
	return pFileData;
#endif

#ifdef MO_POSIX
	int fd = open(filePath, O_RDONLY, 0666);
	if (fd == -1) {
		mo_logf(pContext, "Could not find file: %s", filePath);
		return NULL;
	}
	
	struct stat info;
	if (fstat(fd, &info) == -1) {
		mo_logf(pContext, "Failed to retrieve file info: %s", filePath);
		close(fd);
		return NULL;
	}
	
	if (info.st_size + extraBytes > 0xFFFFFFFF) {
		mo_logf(pContext, "File is too large: %s", filePath);
		close(fd);
		return NULL;	// File is too big.
	}
	
	void* pFileData = mo_malloc(info.st_size + extraBytes);
	if (pFileData == NULL) {
		mo_logf(pContext, "Not enough memory to load image: %s", filePath);
		close(fd);
		return NULL;
	}
	
	ssize_t bytesRead = read(fd, pFileData, info.st_size);
	if (bytesRead != info.st_size) {
		mo_logf(pContext, "Failed to read image file: %s", filePath);
		mo_free(pFileData);
		close(fd);
		return NULL;
	}
	
	close(fd);
	
	if (pFileSizeOut) *pFileSizeOut = (unsigned int)info.st_size;
	return pFileData;
#endif
}

static void* mo_open_and_read_file(mo_context* pContext, const char* filePath, size_t* pFileSizeOut)
{
    return mo_open_and_read_file_with_extra_data(pContext, filePath, pFileSizeOut, 0);
}

mo_result mo_image_create(mo_context* pContext, unsigned int width, unsigned int height, mo_image_format format, const void* pData, mo_image** ppImage)
{
	if (ppImage == NULL) return MO_INVALID_ARGS;
	mo_zero_object(ppImage);
	
	if (pContext == NULL || width == 0 || height == 0 || pData == NULL) return MO_INVALID_ARGS;

	// Allocate the image first so we have an output buffer.
	size_t dataSize = width * height;
	mo_image* pImage = (mo_image*)mo_calloc((sizeof(*pImage)-1) + dataSize);
	if (pImage == NULL) {
		return MO_OUT_OF_MEMORY;
	}
	
	pImage->width = width;
	pImage->height = height;
	pImage->format = format;
	
	switch (format)
	{
		case mo_image_format_rgba8:
		{
			mo_color_rgba colorIn;
			
			const mo_uint8* pRunningPixel = (const mo_uint8*)pData;
			for (unsigned int y = 0; y < height; ++y) {
				for (unsigned int x = 0; x < width; ++x) {
					colorIn.r = pRunningPixel[0];
					colorIn.g = pRunningPixel[1];
					colorIn.b = pRunningPixel[2];
					colorIn.a = pRunningPixel[3];
					mo_color_index colorIndex = mo_find_closest_color(pContext, colorIn);
					
					// Set the transparent bit.
					if (colorIn.a < 255) {
						colorIndex = pContext->profile.transparentColorIndex;
					}
					
					pImage->pData[y*width + x] = colorIndex;
					pRunningPixel += 4;
				}
			}
		} break;
		
		case mo_image_format_native:
		{
			mo_copy_memory(pImage->pData, pData, dataSize);
		} break;
		
		default:
		{
			return MO_UNSUPPORTED_IMAGE_FORMAT;
		} break;
	}
	
	*ppImage = pImage;
	return MO_SUCCESS;
}

static const void* mo_image_load__native(const void* pFileData, size_t fileSize, unsigned int* pWidthOut, unsigned int* pHeightOut, mo_image_format* pFormat)
{
    // The native image format is simple:
    // [4 bytes] FOURCC 'MOI1' (0x31494F4D)
    // [4 bytes] Width
    // [4 bytes] Height
    // [Width x Height bytes] Pixel data as 8-bit color indices. 1 byte per pixel, tightly packed, top down.

    if (pWidthOut  != NULL) *pWidthOut  = 0;
    if (pHeightOut != NULL) *pHeightOut = 0;
    if (pFormat    != NULL) *pFormat    = mo_image_format_unknown;
    if (pFileData == NULL || fileSize < 12) return NULL;

    const mo_uint8* pFileData8 = (const mo_uint8*)pFileData;

    mo_uint32 fourcc;
    mo_copy_memory(&fourcc, pFileData8 + 0, 4);
    if (fourcc != 0x31494F4D) {
        return NULL;    // Not a native image format.
    }

    mo_uint32 width;
    mo_uint32 height;
    mo_copy_memory(&width, pFileData8 + 4, 4);
    mo_copy_memory(&height, pFileData8 + 8, 4);

    if (pWidthOut  != NULL) *pWidthOut  = (unsigned int)width;
    if (pHeightOut != NULL) *pHeightOut = (unsigned int)height;
    if (pFormat    != NULL) *pFormat    = mo_image_format_native;
    return pFileData8 + 12;
}

static void* mo_image_load__tga(const void* pFileData, size_t fileSize, unsigned int* pWidthOut, unsigned int* pHeightOut, mo_image_format* pFormat)
{
    if (pWidthOut  != NULL) *pWidthOut  = 0;
    if (pHeightOut != NULL) *pHeightOut = 0;
    if (pFormat    != NULL) *pFormat    = mo_image_format_unknown;
    if (pFileData == NULL || fileSize < 18) return NULL;

    const mo_uint8* pFileData8 = (const mo_uint8*)pFileData;

    mo_uint8 idlen;
    mo_uint8 colormapType;
    mo_uint8 imageDataType;
    mo_int16 colormapOrigin;
    mo_int16 colormapLength;
    mo_uint8 colormapDepth;
    mo_int16 originX;
    mo_int16 originY;
    mo_int16 width;
    mo_int16 height;
    mo_uint8 bitsPerPixel;
    mo_uint8 descriptor;
    mo_copy_memory(&idlen,          pFileData8 + 0,  1);
    mo_copy_memory(&colormapType,   pFileData8 + 1,  1);
    mo_copy_memory(&imageDataType,  pFileData8 + 2,  1);
    mo_copy_memory(&colormapOrigin, pFileData8 + 3,  2);
    mo_copy_memory(&colormapLength, pFileData8 + 5,  2);
    mo_copy_memory(&colormapDepth,  pFileData8 + 7,  1);
    mo_copy_memory(&originX,        pFileData8 + 8,  2);
    mo_copy_memory(&originY,        pFileData8 + 10, 2);
    mo_copy_memory(&width,          pFileData8 + 12, 2);
    mo_copy_memory(&height,         pFileData8 + 14, 2);
    mo_copy_memory(&bitsPerPixel,   pFileData8 + 16, 1);
    mo_copy_memory(&descriptor,     pFileData8 + 17, 1);

    mo_uint32 flipY = (descriptor >> 5) & 1;

    mo_uint8 alphaMask = 0;
    if ((descriptor & 0x0F) == 0 || bitsPerPixel == 15 || colormapDepth == 15) {
        alphaMask = 0xFF;
    }

    // It's possible for the bits per pixel to be 15. In this case it appears that the data is still stored
    // as 16-bits in the file, but the the alpha bit is simply not used (everything is opaque).
    mo_uint8 bytesPerPixel = bitsPerPixel/8;
    if (bitsPerPixel == 15) {
        bytesPerPixel = 2;
    }

    // Go past the id.
    if (fileSize < 18U + idlen) {
        return NULL;
    }

    pFileData8 += 18+idlen; fileSize -= 18+idlen;
    
    // The colormap.
    const mo_uint8* pColormap8 = pFileData8;
    if (colormapType != 0) {
        size_t bytesToSkip = colormapLength * (colormapDepth/8);
        if (fileSize < bytesToSkip) {
            return NULL;
        }
        pFileData8 += bytesToSkip; fileSize -= bytesToSkip;
    }

    mo_uint8 colormapBytesPerPixel = colormapDepth/8;
    if (colormapDepth == 15) {
        colormapBytesPerPixel = 2;
    }


    mo_uint8* pImageData = (mo_uint8*)mo_malloc(width*height*4);
    if (pImageData == NULL) {
        return NULL;    // Out of memory :(
    }

    switch (imageDataType)
    {
        case 1:     // Uncompressed colormapped.
        {
            if (bytesPerPixel != 1 && bytesPerPixel != 2) {
                goto free_and_return_null;
            }

            size_t srcRowSizeInBytes = width*bytesPerPixel;
            if (fileSize < srcRowSizeInBytes*height) {
                goto free_and_return_null;  // File is too small.
            }

            for (mo_int16 y = 0; y < height; ++y) {
                mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                for (mo_int16 x = 0; x < width; ++x) {
                    mo_uint16 colorIndex = colormapOrigin;
                    if (bytesPerPixel == 1) {
                        colorIndex += pFileData8[0];
                    } else if (bytesPerPixel == 2) {
                        colorIndex += ((mo_uint16*)pFileData8)[0];
                    }

                    pFileData8 += bytesPerPixel;

                    // Grab the color from the colormap.
                    mo_uint8 r = 0;
                    mo_uint8 g = 0;
                    mo_uint8 b = 0;
                    mo_uint8 a = 255;
                    if (colormapBytesPerPixel == 4) {
                        r = pColormap8[colorIndex*4 + 2];
                        g = pColormap8[colorIndex*4 + 1];
                        b = pColormap8[colorIndex*4 + 0];
                        a = pColormap8[colorIndex*4 + 3] | alphaMask;
                    } else if (colormapBytesPerPixel == 3) {
                        r = pColormap8[colorIndex*3 + 2];
                        g = pColormap8[colorIndex*3 + 1];
                        b = pColormap8[colorIndex*3 + 0];
                    } else if (colormapBytesPerPixel == 2) {
                        mo_uint8 b0 = pColormap8[colorIndex*2 + 0];
                        mo_uint8 b1 = pColormap8[colorIndex*2 + 1];
                        r =  (((b1 & 0x7C) >> 2)                       * 255) / 31;
                        g = ((((b0 & 0xE0) >> 5) | ((b1 & 0x03) << 3)) * 255) / 31;
                        b =   ((b0 & 0x1F)                             * 255) / 31;
                        a =  (((b1 & 0x80) >> 7)                       * 255) | alphaMask;
                    }

                    pRow[(x*4)+0] = r;
                    pRow[(x*4)+1] = g;
                    pRow[(x*4)+2] = b;
                    pRow[(x*4)+3] = a;
                }
            }
        } break;

        case 2:     // Uncompressed RGB(A)
        {
            size_t srcRowSizeInBytes = width*bytesPerPixel;
            if (fileSize < srcRowSizeInBytes*height) {
                goto free_and_return_null;  // File is too small.
            }

            // We should now be sitting on the image data.
            if (bytesPerPixel == 4) {
                for (mo_int16 y = 0; y < height; ++y) {
                    mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                    for (mo_int16 x = 0; x < width; ++x) {
                        pRow[(x*4)+0] = pFileData8[(x*4)+2];
                        pRow[(x*4)+1] = pFileData8[(x*4)+1];
                        pRow[(x*4)+2] = pFileData8[(x*4)+0];
                        pRow[(x*4)+3] = pFileData8[(x*4)+3] | alphaMask;
                    }

                    pFileData8 += srcRowSizeInBytes;
                }
            } else if (bytesPerPixel == 3) {
                for (mo_int16 y = 0; y < height; ++y) {
                    mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                    for (mo_int16 x = 0; x < width; ++x) {
                        pRow[(x*4)+0] = pFileData8[(x*3)+2];
                        pRow[(x*4)+1] = pFileData8[(x*3)+1];
                        pRow[(x*4)+2] = pFileData8[(x*3)+0];
                        pRow[(x*4)+3] = 0xFF;
                    }

                    pFileData8 += srcRowSizeInBytes;
                }
            } else if (bytesPerPixel == 2) {
                for (mo_int16 y = 0; y < height; ++y) {
                    mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                    for (mo_int16 x = 0; x < width; ++x) {
                        mo_uint8 b0 = pFileData8[(x*2)+0];
                        mo_uint8 b1 = pFileData8[(x*2)+1];
                        pRow[(x*4)+0] =  (((b1 & 0x7C) >> 2)                       * 255) / 31;
                        pRow[(x*4)+1] = ((((b0 & 0xE0) >> 5) | ((b1 & 0x03) << 3)) * 255) / 31;
                        pRow[(x*4)+2] =   ((b0 & 0x1F)                             * 255) / 31;
                        pRow[(x*4)+3] =  (((b1 & 0x80) >> 7)                       * 255) | alphaMask;
                    }

                    pFileData8 += srcRowSizeInBytes;
                }
            } else {
                goto free_and_return_null;  // Unsupported format.
            }
        } break;

        case 3:     // Uncompressed black and white. Not yet supported.
        {
            size_t srcRowSizeInBytes = width*bytesPerPixel;
            if (fileSize < srcRowSizeInBytes*height) {
                goto free_and_return_null;  // File is too small.
            }

            for (mo_int16 y = 0; y < height; ++y) {
                mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                for (mo_int16 x = 0; x < width; ++x) {
                    mo_uint8 c = pFileData8[x+0];
                    pRow[(x*4)+0] = c;
                    pRow[(x*4)+1] = c;
                    pRow[(x*4)+2] = c;
                    pRow[(x*4)+3] = 255;
                }

                pFileData8 += srcRowSizeInBytes;
            }
        } break;

        case 9:     // RLE colormapped.
        {
            if (bytesPerPixel != 1 && bytesPerPixel != 2) {
                goto free_and_return_null;
            }

            mo_int32 i = 0;
            while (i < width*height) {
                mo_uint8 rle = *pFileData8++;
                mo_uint8 count = (rle & 0x7F) + 1;
                if (rle & 0x80) {
                    mo_uint16 colorIndex = colormapOrigin;
                    if (bytesPerPixel == 1) {
                        colorIndex += pFileData8[0];
                    } else if (bytesPerPixel == 2) {
                        colorIndex += ((mo_uint16*)pFileData8)[0];
                    }

                    pFileData8 += bytesPerPixel;

                    // Grab the color from the colormap.
                    mo_uint8 r = 0;
                    mo_uint8 g = 0;
                    mo_uint8 b = 0;
                    mo_uint8 a = 255;
                    if (colormapBytesPerPixel == 4) {
                        r = pColormap8[colorIndex*4 + 2];
                        g = pColormap8[colorIndex*4 + 1];
                        b = pColormap8[colorIndex*4 + 0];
                        a = pColormap8[colorIndex*4 + 3] | alphaMask;
                    } else if (colormapBytesPerPixel == 3) {
                        r = pColormap8[colorIndex*3 + 2];
                        g = pColormap8[colorIndex*3 + 1];
                        b = pColormap8[colorIndex*3 + 0];
                    } else if (colormapBytesPerPixel == 2) {
                        mo_uint8 b0 = pColormap8[colorIndex*2 + 0];
                        mo_uint8 b1 = pColormap8[colorIndex*2 + 1];
                        r =  (((b1 & 0x7C) >> 2)                       * 255) / 31;
                        g = ((((b0 & 0xE0) >> 5) | ((b1 & 0x03) << 3)) * 255) / 31;
                        b =   ((b0 & 0x1F)                             * 255) / 31;
                        a =  (((b1 & 0x80) >> 7)                       * 255) | alphaMask;
                    }

                    for (mo_uint8 j = 0; j < count; ++j) {
                        mo_int16 y = (mo_int16)((i+j) / width);
                        mo_int16 x = (mo_int16)((i+j) - (width * y));
                        mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                        pRow[(x*4)+0] = r;
                        pRow[(x*4)+1] = g;
                        pRow[(x*4)+2] = b;
                        pRow[(x*4)+3] = a;
                    }
                } else {
                    for (mo_uint8 j = 0; j < count; ++j) {
                        mo_uint16 colorIndex = colormapOrigin;
                        if (bytesPerPixel == 1) {
                            colorIndex += pFileData8[0];
                        } else if (bytesPerPixel == 2) {
                            colorIndex += ((mo_uint16*)pFileData8)[0];
                        }

                        pFileData8 += bytesPerPixel;

                        // Grab the color from the colormap.
                        mo_uint8 r = 0;
                        mo_uint8 g = 0;
                        mo_uint8 b = 0;
                        mo_uint8 a = 255;
                        if (colormapBytesPerPixel == 4) {
                            r = pColormap8[colorIndex*4 + 2];
                            g = pColormap8[colorIndex*4 + 1];
                            b = pColormap8[colorIndex*4 + 0];
                            a = pColormap8[colorIndex*4 + 3] | alphaMask;
                        } else if (colormapBytesPerPixel == 3) {
                            r = pColormap8[colorIndex*3 + 2];
                            g = pColormap8[colorIndex*3 + 1];
                            b = pColormap8[colorIndex*3 + 0];
                        } else if (colormapBytesPerPixel == 2) {
                            mo_uint8 b0 = pColormap8[colorIndex*2 + 0];
                            mo_uint8 b1 = pColormap8[colorIndex*2 + 1];
                            r =  (((b1 & 0x7C) >> 2)                       * 255) / 31;
                            g = ((((b0 & 0xE0) >> 5) | ((b1 & 0x03) << 3)) * 255) / 31;
                            b =   ((b0 & 0x1F)                             * 255) / 31;
                            a =  (((b1 & 0x80) >> 7)                       * 255) | alphaMask;
                        }

                        mo_int16 y = (mo_int16)((i+j) / width);
                        mo_int16 x = (mo_int16)((i+j) - (width * y));
                        mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                        pRow[(x*4)+0] = r;
                        pRow[(x*4)+1] = g;
                        pRow[(x*4)+2] = b;
                        pRow[(x*4)+3] = a;
                    }
                }

                i += count;
            }
        } break;

        case 10:    // RLE RGB(A).
        {
            // RLE is a bit more annoying than uncompressed RGB so we'll do it in a slightly more inefficient way and just
            // do it all in a single loop.
            mo_int32 i = 0;
            while (i < width*height) {
                mo_uint8 rle = *pFileData8++;
                mo_uint8 count = (rle & 0x7F) + 1;
                if (rle & 0x80) {
                    mo_uint8 r = 0;
                    mo_uint8 g = 0;
                    mo_uint8 b = 0;
                    mo_uint8 a = 255;
                    if (bytesPerPixel == 4) {
                        r = pFileData8[2];
                        g = pFileData8[1];
                        b = pFileData8[0];
                        a = pFileData8[3];
                    } else if (bytesPerPixel == 3) {
                        r = pFileData8[2];
                        g = pFileData8[1];
                        b = pFileData8[0];
                    } else if (bytesPerPixel == 2) {
                        mo_uint8 b0 = pFileData8[0];
                        mo_uint8 b1 = pFileData8[1];
                        r =  (((b1 & 0x7C) >> 2)                       * 255) / 31;
                        g = ((((b0 & 0xE0) >> 5) | ((b1 & 0x03) << 3)) * 255) / 31;
                        b =   ((b0 & 0x1F)                             * 255) / 31;
                        a =  (((b1 & 0x80) >> 7)                       * 255) | alphaMask;
                    }
                    pFileData8 += bytesPerPixel;
                    
                    for (mo_uint8 j = 0; j < count; ++j) {
                        mo_int16 y = (mo_int16)((i+j) / width);
                        mo_int16 x = (mo_int16)((i+j) - (width * y));
                        mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                        pRow[(x*4)+0] = r;
                        pRow[(x*4)+1] = g;
                        pRow[(x*4)+2] = b;
                        pRow[(x*4)+3] = a;
                    }
                } else {
                    for (mo_uint8 j = 0; j < count; ++j) {
                        mo_uint8 r = 0;
                        mo_uint8 g = 0;
                        mo_uint8 b = 0;
                        mo_uint8 a = 255;
                        if (bytesPerPixel == 4) {
                            r = pFileData8[2];
                            g = pFileData8[1];
                            b = pFileData8[0];
                            a = pFileData8[3];
                        } else if (bytesPerPixel == 3) {
                            r = pFileData8[2];
                            g = pFileData8[1];
                            b = pFileData8[0];
                        } else if (bytesPerPixel == 2) {
                            mo_uint8 b0 = pFileData8[0];
                            mo_uint8 b1 = pFileData8[1];
                            r =  (((b1 & 0x7C) >> 2)                       * 255) / 31;
                            g = ((((b0 & 0xE0) >> 5) | ((b1 & 0x03) << 3)) * 255) / 31;
                            b =   ((b0 & 0x1F)                             * 255) / 31;
                            a =  (((b1 & 0x80) >> 7)                       * 255) | alphaMask;
                        }
                        pFileData8 += bytesPerPixel;

                        mo_int16 y = (mo_int16)((i+j) / width);
                        mo_int16 x = (mo_int16)((i+j) - (width * y));
                        mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                        pRow[(x*4)+0] = r;
                        pRow[(x*4)+1] = g;
                        pRow[(x*4)+2] = b;
                        pRow[(x*4)+3] = a;
                    }
                }

                i += count;
            }
        } break;

        case 11:    // RLE compressed black and white.
        {
            if (bitsPerPixel != 8) {
                goto free_and_return_null;
            }

            mo_int32 i = 0;
            while (i < width*height) {
                mo_uint8 rle = *pFileData8++;
                mo_uint8 count = (rle & 0x7F) + 1;
                if (rle & 0x80) {
                    mo_uint8 c = pFileData8[0];
                    pFileData8 += bytesPerPixel;
                    
                    for (mo_uint8 j = 0; j < count; ++j) {
                        mo_int16 y = (mo_int16)((i+j) / width);
                        mo_int16 x = (mo_int16)((i+j) - (width * y));
                        mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                        pRow[(x*4)+0] = c;
                        pRow[(x*4)+1] = c;
                        pRow[(x*4)+2] = c;
                        pRow[(x*4)+3] = 255;
                    }
                } else {
                    for (mo_uint8 j = 0; j < count; ++j) {
                        mo_uint8 c = pFileData8[0];
                        pFileData8 += bytesPerPixel;

                        mo_int16 y = (mo_int16)((i+j) / width);
                        mo_int16 x = (mo_int16)((i+j) - (width * y));
                        mo_uint8* pRow = (flipY) ? (pImageData + (y*width*4)) : (pImageData + ((height-y-1)*width*4));
                        pRow[(x*4)+0] = c;
                        pRow[(x*4)+1] = c;
                        pRow[(x*4)+2] = c;
                        pRow[(x*4)+3] = 255;
                    }
                }

                i += count;
            }
        } break;
    }

    

    if (pWidthOut  != NULL) *pWidthOut  = (unsigned int)width;
    if (pHeightOut != NULL) *pHeightOut = (unsigned int)height;
    if (pFormat    != NULL) *pFormat    = mo_image_format_rgba8;
    return (void*)pImageData;

free_and_return_null:
    mo_free(pImageData);
    return NULL;
}

#ifdef MO_HAS_STB_IMAGE
static void* mo_image_load__stb(const void* pFileData, size_t fileSize, unsigned int* pWidthOut, unsigned int* pHeightOut, mo_image_format* pFormat)
{
    if (pWidthOut  != NULL) *pWidthOut  = 0;
    if (pHeightOut != NULL) *pHeightOut = 0;
    if (pFormat    != NULL) *pFormat    = mo_image_format_unknown;
    if (pFileData  == NULL) return NULL;

    int widthSTB;
	int heightSTB;
	stbi_uc* pImageData = stbi_load_from_memory(pFileData, (int)fileSize, &widthSTB, &heightSTB, NULL, 4);
	if (pImageData == NULL) {
		return NULL;
	}

    if (pWidthOut  != NULL) *pWidthOut  = (unsigned int)widthSTB;
    if (pHeightOut != NULL) *pHeightOut = (unsigned int)heightSTB;
    if (pFormat    != NULL) *pFormat    = mo_image_format_rgba8;
    return (void*)pImageData;
}
#endif

mo_result mo_image_load(mo_context* pContext, const char* filePath, mo_image** ppImage)
{
	if (ppImage == NULL) return MO_INVALID_ARGS;
	mo_zero_object(ppImage);
	
	if (pContext == NULL || filePath == NULL) return MO_INVALID_ARGS;
	
	size_t fileSize;
	void* pFileData = mo_open_and_read_file(pContext, filePath, &fileSize);
	if (pFileData == NULL) {
		return MO_DOES_NOT_EXIST;
	}
	
	unsigned int width = 0;
	unsigned int height = 0;
	mo_image_format format = mo_image_format_unknown;
	const void* pImageData = NULL;
    void* pImageDataTGA = NULL;
#ifdef MO_HAS_STB_IMAGE
    void* pImageDataSTB = NULL;
#endif

	if (mo_extension_equal(filePath, "moimage")) {
        pImageData = mo_image_load__native(pFileData, fileSize, &width, &height, &format);
        if (pImageData == NULL) {
            mo_logf(pContext, "Corrupt image file (%s)", filePath);
            mo_free(pFileData);
            return MO_INVALID_RESOURCE;
        }
	} else if (mo_extension_equal(filePath, "tga")) {
        pImageDataTGA = mo_image_load__tga(pFileData, fileSize, &width, &height, &format);
        if (pImageDataTGA == NULL) {
            mo_logf(pContext, "Corrupt image file (%s)", filePath);
            mo_free(pFileData);
            return MO_INVALID_RESOURCE;
        }

        pImageData = pImageDataTGA;
    } else {
#ifdef MO_HAS_STB_IMAGE
        pImageDataSTB = mo_image_load__stb(pFileData, fileSize, &width, &height, &format);
        if (pImageDataSTB == NULL) {
            mo_logf(pContext, "Unsupported or corrupt image file (%s): %s", filePath, stbi__g_failure_reason);
            mo_free(pFileData);
			return MO_INVALID_RESOURCE;
        }

        pImageData = pImageDataSTB;
#else
		return MO_INVALID_RESOURCE;
#endif
	}
	
	mo_result result = mo_image_create(pContext, width, height, format, pImageData, ppImage);

    if (pImageDataTGA) {
        mo_free(pImageDataTGA);
    }

#ifdef MO_HAS_STB_IMAGE
    if (pImageDataSTB) {
	    stbi_image_free(pImageDataSTB);
    }
#endif
	
    mo_free(pFileData); // <-- This must be done last! The reason is that pImageData might just be an offset of this pointer.
	return result;
}

void mo_image_delete(mo_context* pContext, mo_image* pImage)
{
	if (pContext == NULL || pImage == NULL) return;
	mo_free(pImage);
}


//// Drawing ////

static float mo_color_distance2(mo_color_rgba c1, mo_color_rgba c2)
{
    // This is just simple Euclidean distance. Can probably improve the accuracy of this later on.
    int diffr = (int)c2.r - (int)c1.r;
    int diffg = (int)c2.g - (int)c1.g;
    int diffb = (int)c2.b - (int)c1.b;
    return (float)(diffr*diffr + diffg*diffg + diffb*diffb);
}

mo_color_index mo_find_closest_color(mo_context* pContext, mo_color_rgba color)
{
	if (pContext == NULL) return 0;
	
	// We just do a simple distance test. There's no need for anything advanced here, but we do use YUV for the distance.
    float minDistance = 3.402823e+38f;  // FLT_MAX
    mo_color_index closestIndex = 0;

    mo_color_index i = 0;
    for (;;) {
        if (i != pContext->profile.transparentColorIndex) {
            float distance = mo_color_distance2(color, pContext->profile.palette[i]);
            if (minDistance > distance) {
                minDistance = distance;
                closestIndex = i;

                if (minDistance == 0) {
                    break;
                }
            }
        }

        mo_assert(pContext->profile.paletteSize > 0);
        if (i == 255 || i == pContext->profile.paletteSize-1) {
            break;
        }

        i += 1;
    }

	return closestIndex;
}

void mo_clear(mo_context* pContext, mo_color_index colorIndex)
{
	if (pContext == NULL) return;

	for (unsigned int y = 0; y < pContext->profile.resolutionY; ++y) {
		for (unsigned int x = 0; x < pContext->profile.resolutionX; ++x) {
			pContext->screen[y*pContext->profile.resolutionX + x] = colorIndex;
		}
	}
}

void mo_draw_quad(mo_context* pContext, int posX, int posY, int sizeX, int sizeY, mo_color_index colorIndex)
{
	if (pContext == NULL) return;
	
	int left   = posX;
	int top    = posY;
	int right  = left + sizeX;
	int bottom = top + sizeY;
	
	// Is the quad entirely out of bounds?
	if (right < 0 || bottom < 0) return;
	if (left >= (int)pContext->profile.resolutionX || top >= (int)pContext->profile.resolutionY) return;
	
	// Clamp.
	if (left   < 0)   left   = 0;
	if (top    < 0)   top    = 0;
	if (right  > (int)pContext->profile.resolutionX) right  = (int)pContext->profile.resolutionX;
	if (bottom > (int)pContext->profile.resolutionY) bottom = (int)pContext->profile.resolutionY;
	
	// Draw.
	// TODO: Optimize me.
	for (int y = top; y < bottom; ++y) {
		for (int x = left; x < right; ++x) {
			pContext->screen[y*pContext->profile.resolutionX + x] = colorIndex;
		}
	}
}

static mo_uint8 pFontData[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 
	0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 
	0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void mo_draw_glyph(mo_context* pContext, int posX, int posY, char c, mo_color_index colorIndex)
{
	c -= 16;
	if (c < 0 || c > 111) c = 0;
	
	const unsigned int glyphSizeX = 9;
	const unsigned int glyphSizeY = 9;
	unsigned int offset = (c * (glyphSizeX*glyphSizeY));
	
	int left   = posX;
	int top    = posY;
	int right  = left + glyphSizeX;
	int bottom = top + glyphSizeY;
	
	int imageXOffset = 0;
	int imageYOffset = 0;
	
	// Is the quad entirely out of bounds?
	if (right < 0 || bottom < 0) return;
	if (left >= (int)pContext->profile.resolutionX || top >= (int)pContext->profile.resolutionY) return;
	
	// Clamp.
	if (left < 0) {
		imageXOffset = -left;
		left = 0;
	}
	if (top < 0) {
		imageYOffset = -top;
		top = 0;
	}
	
	if (right  > (int)pContext->profile.resolutionX) right  = (int)pContext->profile.resolutionX;
	if (bottom > (int)pContext->profile.resolutionY) bottom = (int)pContext->profile.resolutionY;
	
	// Draw.
	// TODO: Optimize me.
	for (int y = top; y < bottom; ++y) {
		for (int x = left; x < right; ++x) {
			int glyphX = imageXOffset + (x - left);
			int glyphY = imageYOffset + (y - top);
			mo_uint8 fontPixel = (pFontData + offset)[glyphY*glyphSizeX + glyphX];
			if (fontPixel != 0x00) {
				pContext->screen[y*pContext->profile.resolutionX + x] = colorIndex;
			}
		}
	}
}

void mo_draw_text(mo_context* pContext, int posX, int posY, mo_color_index colorIndex, const char* text)
{
	if (pContext == NULL) return;
	
	int penPosX = posX;
	int penPosY = posY;
	while (*text != '\0') {
		mo_draw_glyph(pContext, penPosX, penPosY, *text, colorIndex);
		
		penPosX += 9;
		text += 1;
	}
}

void mo_draw_textf(mo_context* pContext, int posX, int posY, mo_color_index colorIndex, const char* format, ...)
{
    va_list args;
	
    va_start(args, format);
#if defined(_MSC_VER)
    int len = _vscprintf(format, args);
#else
    int len = vsnprintf(NULL, 0, format, args);
#endif
	va_end(args);
	
    if (len < 0) {
        return;
    }

    char* text = (char*)mo_malloc(len+1);
    if (text == NULL) {
        va_end(args);
        return;
    }

	va_start(args, format);
#if defined(_MSC_VER)
    len = vsprintf_s(text, len+1, format, args);
#else
    len = vsnprintf(text, len+1, format, args);
#endif
	va_end(args);

    mo_draw_text(pContext, posX, posY, colorIndex, text);

    mo_free(text);
    va_end(args);
}

void mo_draw_image(mo_context* pContext, int dstX, int dstY, mo_image* pImage, int srcX, int srcY, int srcWidth, int srcHeight)
{
	if (pImage == NULL) return;
    mo_draw_image_scaled(pContext, dstX, dstY, pImage->width, pImage->height, pImage, srcX, srcY, srcWidth, srcHeight);
}

void mo_draw_image_scaled(mo_context* pContext, int dstX, int dstY, int dstWidth, int dstHeight, mo_image* pImage, int srcX, int srcY, int srcWidth, int srcHeight/*, float rotation*/)
{
    if (pContext == NULL || pImage == NULL) return;
    //mo_draw_quad(pContext, x, y, pImage->width, pImage->height, 3);   // Debugging

    //mo_assert(rotation == 0);   // Rotation isn't supported yet.

#if 1
    // If you trigger any of these asserts it means you have an error in your sub-imaging logic.
    mo_assert(srcWidth  > 0);
    mo_assert(srcHeight > 0);
    mo_assert(srcX >= 0);
    mo_assert(srcY >= 0);
    mo_assert(srcX+srcWidth  <= (int)pImage->width);
    mo_assert(srcY+srcHeight <= (int)pImage->height);
#endif

    // Make sure inputs are clamped.
#if 1
    if (srcX < 0) {
        srcWidth += srcX;
        srcX = 0;
    }
    if (srcY < 0) {
        srcHeight += srcY;
        srcY = 0;
    }
    
    if (srcWidth+srcX > (int)pImage->width) {
        srcWidth = pImage->width - srcX;
    }
    if (srcHeight+srcY > (int)pImage->height) {
        srcHeight = pImage->height - srcY;
    }
#endif

    float scaleX = (float)srcWidth / dstWidth;
    float scaleY = (float)srcHeight / dstHeight;

	// Is the quad entirely out of bounds?
	if (dstX+dstWidth < 0 || dstY+dstHeight < 0) return;
	if (dstX >= (int)pContext->profile.resolutionX || dstY >= (int)pContext->profile.resolutionY) return;
	
	// Clamp.
    float srcXOffset = 0;
    float srcYOffset = 0;
	if (dstX < 0) {
        srcXOffset = -dstX*scaleX;
        dstWidth += dstX;
		dstX = 0;
	}
	if (dstY < 0) {
        srcYOffset = -dstY*scaleY;
        dstHeight += dstY;
		dstY = 0;
	}
	
	if (dstWidth+dstX > (int)pContext->profile.resolutionX) {
        dstWidth = (int)pContext->profile.resolutionX - dstX;
    }
	if (dstHeight+dstY > (int)pContext->profile.resolutionY) {
        dstHeight = (int)pContext->profile.resolutionY - dstY;
    }
	
    //if (rotation == 0) {
        if (scaleX == 1.0f && scaleY == 1.0f) {
            // No rotation, no scaling. Fast path.
            // TODO: Optimize me.
	        for (int y = 0; y < dstHeight; ++y) {
		        for (int x = 0; x < dstWidth; ++x) {
			        int imageX = (int)(srcXOffset + srcX + x);
			        int imageY = (int)(srcYOffset + srcY + y);

                    mo_color_index colorIndex = pImage->pData[imageY*pImage->width + imageX];
                    if (colorIndex != pContext->profile.transparentColorIndex) {
			            pContext->screen[(dstY+y)*pContext->profile.resolutionX + (dstX+x)] = colorIndex;
                    }
		        }
	        }
        } else {
            // No rotation, with scaling.
            // TODO: Optimize me.
            for (int y = 0; y < dstHeight; ++y) {
                for (int x = 0; x < dstWidth; ++x) {
                    int imageX = (int)(srcXOffset + srcX + (x * scaleX));
                    int imageY = (int)(srcYOffset + srcY + (y * scaleY));

                    mo_color_index colorIndex = pImage->pData[imageY*pImage->width + imageX];
                    if (colorIndex != pContext->profile.transparentColorIndex) {
                        pContext->screen[(dstY+y)*pContext->profile.resolutionX + (dstX+x)] = colorIndex;
                    }
                }
            }
        }
    //} else {
        // It's rotated. We do rotation and scaling on the same path for now.

    //}
}


//// Audio ////

mo_result mo_sound_source_create(mo_context* pContext, unsigned int channels, unsigned int sampleRate, mo_uint64 sampleCount, const mo_int16* pSampleData, mo_sound_source** ppSource)
{
    if (ppSource == NULL) return MO_INVALID_ARGS;
	mo_zero_object(ppSource);

	if (pContext == NULL || channels == 0 || sampleRate == 0 || sampleCount == 0) return MO_INVALID_ARGS;
    if (sampleCount > SIZE_MAX/sizeof(mo_int16)) return MO_INVALID_ARGS; // Sound is too big.
	
    size_t sampleDataSize = (size_t)(sampleCount * sizeof(mo_int16));

    mo_sound_source* pSource = (mo_sound_source*)mo_calloc(sizeof(*pSource) + sampleDataSize);
    if (pSource == NULL) {
        return MO_OUT_OF_MEMORY;
    }

    pSource->channels = channels;
    pSource->sampleRate = sampleRate;
    pSource->sampleCount = sampleCount;
    mo_copy_memory(pSource->pSampleData, pSampleData, sampleDataSize);

    *ppSource = pSource;
	return MO_SUCCESS;
}

static mo_int16* mo_sound_source_load__wav(const void* pFileData, size_t fileSize, unsigned int* pChannels, unsigned int* pSampleRate, mo_uint64* pSampleCount)
{
    // NOTES:
    // - This function only works on little endian.
    // - This should work seamlessly with Wave64.

    const mo_uint8* pFileData8 = (const mo_uint8*)pFileData;

    // These are #undef-ed at the end of this function.
    #define MO_WAV_MALLOC(sz)           mo_malloc(sz)   //malloc(sz)
    #define MO_WAV_FREE(p)              mo_free(p)      //free(p)
    #define MO_WAV_COPY(dst, src, sz)   mo_copy_memory(dst, src, sz) //do { for (size_t i = 0; i < (sz); ++i) { ((unsigned char*)(dst))[i] = ((unsigned char*)(src))[i]; } } while (0)    
    #define MO_WAV_SEEK(offset)         (pFileData8 += (offset), fileSize -= (size_t)(offset))
    #define MO_WAV_READ_UINT8()         (*(unsigned char*     )(pFileData8)); MO_WAV_SEEK(1)
    #define MO_WAV_READ_UINT16()        (*(unsigned short*    )(pFileData8)); MO_WAV_SEEK(2)
    #define MO_WAV_READ_UINT32()        (*(unsigned int*      )(pFileData8)); MO_WAV_SEEK(4)
    #define MO_WAV_READ_UINT64()        (*(unsigned long long*)(pFileData8)); MO_WAV_SEEK(8)
    #define MO_WAV_READ_GUID(guid)      MO_WAV_COPY(guid, pFileData8, 16); MO_WAV_SEEK(16)
    #define MO_WAV_GUID_EQUAL(a, b)     ((mo_bool32)(((((mo_uint64*)(a))[0] == ((mo_uint64*)(b))[0]) && (((mo_uint64*)(a))[1] == ((mo_uint64*)(b))[1]))))
    
    const unsigned char wavGUID_W64_RIFF[16] = {0x72,0x69,0x66,0x66, 0x2E,0x91, 0xCF,0x11, 0xA5,0xD6, 0x28,0xDB,0x04,0xC1,0x00,0x00};    // 66666972-912E-11CF-A5D6-28DB04C10000
    const unsigned char wavGUID_W64_WAVE[16] = {0x77,0x61,0x76,0x65, 0xF3,0xAC, 0xD3,0x11, 0x8C,0xD1, 0x00,0xC0,0x4F,0x8E,0xDB,0x8A};    // 65766177-ACF3-11D3-8CD1-00C04F8EDB8A
    const unsigned char wavGUID_W64_FMT [16] = {0x66,0x6D,0x74,0x20, 0xF3,0xAC, 0xD3,0x11, 0x8C,0xD1, 0x00,0xC0,0x4F,0x8E,0xDB,0x8A};    // 20746D66-ACF3-11D3-8CD1-00C04F8EDB8A
    const unsigned char wavGUID_W64_DATA[16] = {0x64,0x61,0x74,0x61, 0xF3,0xAC, 0xD3,0x11, 0x8C,0xD1, 0x00,0xC0,0x4F,0x8E,0xDB,0x8A}; 

    if (pChannels) *pChannels = 0;
    if (pSampleRate) *pSampleRate = 0;
    if (pSampleCount) *pSampleCount = 0;
    if (pFileData == NULL || fileSize < 4) return NULL;

    mo_bool32 isWave64 = MO_FALSE;

    mo_uint32 riffFOURCC = MO_WAV_READ_UINT32();
    if (riffFOURCC == 0x46464952) {
        isWave64 = MO_FALSE;
    } else if (riffFOURCC == 0x66666972) {
        isWave64 = MO_TRUE;
        if (fileSize < 12) return NULL;
        for (int i = 0; i < 12; ++i) { if (pFileData8[i] != wavGUID_W64_RIFF[i+4]) return NULL; }
        MO_WAV_SEEK(12);
    } else {
        return NULL;
    }
    
    if (!isWave64) {
        if (fileSize < 8) return NULL;
        mo_uint32 chunkSize  = MO_WAV_READ_UINT32();
        mo_uint32 waveFOURCC = MO_WAV_READ_UINT32();
        if (chunkSize < 36 || waveFOURCC != 0x45564157) {
            return NULL;
        }
    } else {
        if (fileSize < 24) return NULL;
        mo_uint64 chunkSize = MO_WAV_READ_UINT64();
        if (chunkSize < 84) {
            return NULL;
        }

        mo_uint8 waveGUID[16];
        MO_WAV_READ_GUID(waveGUID);
        if (!MO_WAV_GUID_EQUAL(waveGUID, wavGUID_W64_WAVE)) {
            return NULL;
        }
    }


    // Next chunk should always be the "fmt " chunk.
    mo_uint64 fmtSize = 0;
    mo_uint32 fmtPadding = 0;
    if (!isWave64) {
        if (fileSize < 8) return NULL;

        mo_uint32 fmtFOURCC = MO_WAV_READ_UINT32();
        fmtSize = MO_WAV_READ_UINT32();
        fmtPadding = fmtSize % 2;

        if (fmtFOURCC != 0x20746d66) {
            return NULL;
        }
    } else {
        if (fileSize < 24) return NULL;

        mo_uint8 fmtGUID[16];
        MO_WAV_READ_GUID(fmtGUID);
        if (!MO_WAV_GUID_EQUAL(fmtGUID, wavGUID_W64_FMT)) {
            return NULL;
        }

        fmtSize  = MO_WAV_READ_UINT64();
        fmtSize -= 24;  // Subtract 24 because w64 includes the size of the header which is inconsistent with regular WAV.
        fmtPadding = fmtSize % 8;
    }

    if (fileSize < 16) return NULL;

    mo_uint16 formatTag      = MO_WAV_READ_UINT16();
    mo_uint16 channels       = MO_WAV_READ_UINT16();
    mo_uint32 sampleRate     = MO_WAV_READ_UINT32();
    mo_uint32 avgBytesPerSec = MO_WAV_READ_UINT32(); (void)avgBytesPerSec;
    mo_uint16 blockAlign     = MO_WAV_READ_UINT16();
    mo_uint16 bitsPerSample  = MO_WAV_READ_UINT16();

    mo_uint16 extendedSize = 0;
    mo_uint16 validBitsPerSample = 0;
    mo_uint32 channelMask = 0;
    mo_uint8  subformatGUID[16] = {0};
    if (fmtSize > 16) {
        if (fileSize < fmtSize-16) return NULL;
        extendedSize = MO_WAV_READ_UINT16();

        size_t leftover = 0;
        if (extendedSize > 0) {
            if (extendedSize != 22 || fileSize < 22) return NULL; // The extended size should always be 22.
            validBitsPerSample = MO_WAV_READ_UINT16();
            channelMask        = MO_WAV_READ_UINT32();
            MO_WAV_READ_GUID(subformatGUID);
            leftover = (size_t)(fmtSize - 40);
        } else {
            leftover = (size_t)(fmtSize - 18);
        }

        if (fileSize < leftover) return NULL;
        pFileData8 += leftover; fileSize -= leftover;
    }

    if (fileSize < fmtPadding) return NULL;
    pFileData8 += fmtPadding; fileSize -= fmtPadding;

    // The only other chunk we care about now is the "data" chunk. This is not necessarily the next chunk so we need to loop.
    mo_uint64 dataSize;
    mo_uint32 dataPadding;
    for (;;) {
        dataSize = 0;
        dataPadding = 0;
        if (!isWave64) {
            if (fileSize < 8) return NULL;

            mo_uint32 dataFOURCC = MO_WAV_READ_UINT32();
            dataSize = MO_WAV_READ_UINT32();
            dataPadding = dataSize % 2;

            if (dataFOURCC == 0x61746164) {
                break;
            }
        } else {
            if (fileSize < 24) return NULL;

            mo_uint8 dataGUID[16];
            MO_WAV_READ_GUID(dataGUID);

            mo_bool32 isDataChunk = MO_WAV_GUID_EQUAL(dataGUID, wavGUID_W64_DATA);
            dataSize  = MO_WAV_READ_UINT64();
            dataSize -= 24;    // Subtract 24 because w64 includes the size of the header which is inconsistent with regular WAV.
            dataPadding = dataSize % 8;

            if (isDataChunk) {
                break;  // It's the "data" GUID.
            }
        }

        dataSize += dataPadding;
        if (fileSize < dataSize) return NULL;
        MO_WAV_SEEK(dataSize);
    }

    // At this point we should be sitting on the raw sample data.
    mo_uint64 sampleCount = dataSize / (blockAlign / channels);
    if (sampleCount > 0x7FFFFFFF) {
        return NULL;    // File is too big.
    }

    mo_uint16 actualFormatTag = formatTag;
    if (actualFormatTag == 0xFFFE) {
        actualFormatTag = *(mo_uint16*)subformatGUID;   // The actual format tag is derived from the first 2 bytes of the subformat GUID.
    }

    mo_int16* pSamples = (mo_int16*)MO_WAV_MALLOC((size_t)sampleCount * sizeof(mo_int16));
    if (pSamples == NULL) {
        return NULL;
    }

    switch (actualFormatTag)
    {
        case 0x0001:    // WAVE_FORMAT_PCM
        {
            if (bitsPerSample == 16) {
                MO_WAV_COPY(pSamples, pFileData8, (size_t)sampleCount * sizeof(mo_int16));
            } else {
                // 8-, 24- and 32-bit conversions can be optimized.
                if (bitsPerSample == 8) {
                    if (fileSize < sampleCount*1) goto free_and_return_null;
                    for (mo_uint64 i = 0; i < sampleCount; ++i) {
                        pSamples[i] = ((mo_int16)pFileData8[i] - 128) << 8;
                    }
                    pFileData8 += sampleCount*1; fileSize -= (size_t)sampleCount*1;
                } else if (bitsPerSample == 24) {
                    if (fileSize < sampleCount*3) goto free_and_return_null;
                    for (mo_uint64 i = 0; i < sampleCount; ++i) {
                        mo_uint32 s0 = pFileData8[i*3 + 0];
                        mo_uint32 s1 = pFileData8[i*3 + 1];
                        mo_uint32 s2 = pFileData8[i*3 + 2];
                        int32_t sample32 = (int32_t)((s0 << 8) | (s1 << 16) | (s2 << 24));
                        pSamples[i] = sample32 >> 16;
                    }
                    pFileData8 += sampleCount*3; fileSize -= (size_t)sampleCount*3;
                } else if (bitsPerSample == 32) {
                    if (fileSize < sampleCount*4) goto free_and_return_null;
                    for (mo_uint64 i = 0; i < sampleCount; ++i) {
                        pSamples[i] = ((mo_uint32*)pFileData8)[i] >> 16;
                    }
                    pFileData8 += sampleCount*4; fileSize -= (size_t)sampleCount*4;
                } else {
                    // Generic.
                    mo_uint32 bytesPerSample = (mo_uint32)(blockAlign / channels);
                    for (mo_uint64 i = 0; i < sampleCount; ++i) {
                        if (fileSize < bytesPerSample) goto free_and_return_null;

                        mo_int32 sample32 = 0;
                        mo_int32 shift = (8 - bytesPerSample) * 8;
                        for (mo_uint16 j = 0; j < bytesPerSample && j < 4; ++j) {
                            sample32 |= (mo_uint32)(pFileData8[j]) << shift;
                            shift += 8;
                        }

                        pSamples[i] = sample32 >> 16;
                        pFileData8 += bytesPerSample; fileSize -= bytesPerSample;
                    }
                }
            }
        } break;

        case 0x0003:    // WAVE_FORMAT_IEEE_FLOAT
        {
            if (bitsPerSample == 32) {
                for (mo_uint64 i = 0; i < sampleCount; ++i) {
                    if (fileSize < 4) goto free_and_return_null;
                    pSamples[i] = (mo_int16)((*(float*)pFileData8) * 32768.0f);
                    pFileData8 += 4; fileSize -= 4;
                }
            } else if (bitsPerSample == 64) {
                for (mo_uint64 i = 0; i < sampleCount; ++i) {
                    if (fileSize < 8) goto free_and_return_null;
                    pSamples[i] = (mo_int16)((*(double*)pFileData8) * 32768.0);
                    pFileData8 += 8; fileSize -= 8;
                }
            } else {
                goto free_and_return_null;
            }
        } break;

        case 0x0006:    // WAVE_FORMAT_ALAW
        {
            mo_uint16 table[256] = {
                0xEA80, 0xEB80, 0xE880, 0xE980, 0xEE80, 0xEF80, 0xEC80, 0xED80, 0xE280, 0xE380, 0xE080, 0xE180, 0xE680, 0xE780, 0xE480, 0xE580, 
                0xF540, 0xF5C0, 0xF440, 0xF4C0, 0xF740, 0xF7C0, 0xF640, 0xF6C0, 0xF140, 0xF1C0, 0xF040, 0xF0C0, 0xF340, 0xF3C0, 0xF240, 0xF2C0, 
                0xAA00, 0xAE00, 0xA200, 0xA600, 0xBA00, 0xBE00, 0xB200, 0xB600, 0x8A00, 0x8E00, 0x8200, 0x8600, 0x9A00, 0x9E00, 0x9200, 0x9600, 
                0xD500, 0xD700, 0xD100, 0xD300, 0xDD00, 0xDF00, 0xD900, 0xDB00, 0xC500, 0xC700, 0xC100, 0xC300, 0xCD00, 0xCF00, 0xC900, 0xCB00, 
                0xFEA8, 0xFEB8, 0xFE88, 0xFE98, 0xFEE8, 0xFEF8, 0xFEC8, 0xFED8, 0xFE28, 0xFE38, 0xFE08, 0xFE18, 0xFE68, 0xFE78, 0xFE48, 0xFE58, 
                0xFFA8, 0xFFB8, 0xFF88, 0xFF98, 0xFFE8, 0xFFF8, 0xFFC8, 0xFFD8, 0xFF28, 0xFF38, 0xFF08, 0xFF18, 0xFF68, 0xFF78, 0xFF48, 0xFF58, 
                0xFAA0, 0xFAE0, 0xFA20, 0xFA60, 0xFBA0, 0xFBE0, 0xFB20, 0xFB60, 0xF8A0, 0xF8E0, 0xF820, 0xF860, 0xF9A0, 0xF9E0, 0xF920, 0xF960, 
                0xFD50, 0xFD70, 0xFD10, 0xFD30, 0xFDD0, 0xFDF0, 0xFD90, 0xFDB0, 0xFC50, 0xFC70, 0xFC10, 0xFC30, 0xFCD0, 0xFCF0, 0xFC90, 0xFCB0, 
                0x1580, 0x1480, 0x1780, 0x1680, 0x1180, 0x1080, 0x1380, 0x1280, 0x1D80, 0x1C80, 0x1F80, 0x1E80, 0x1980, 0x1880, 0x1B80, 0x1A80, 
                0x0AC0, 0x0A40, 0x0BC0, 0x0B40, 0x08C0, 0x0840, 0x09C0, 0x0940, 0x0EC0, 0x0E40, 0x0FC0, 0x0F40, 0x0CC0, 0x0C40, 0x0DC0, 0x0D40, 
                0x5600, 0x5200, 0x5E00, 0x5A00, 0x4600, 0x4200, 0x4E00, 0x4A00, 0x7600, 0x7200, 0x7E00, 0x7A00, 0x6600, 0x6200, 0x6E00, 0x6A00, 
                0x2B00, 0x2900, 0x2F00, 0x2D00, 0x2300, 0x2100, 0x2700, 0x2500, 0x3B00, 0x3900, 0x3F00, 0x3D00, 0x3300, 0x3100, 0x3700, 0x3500, 
                0x0158, 0x0148, 0x0178, 0x0168, 0x0118, 0x0108, 0x0138, 0x0128, 0x01D8, 0x01C8, 0x01F8, 0x01E8, 0x0198, 0x0188, 0x01B8, 0x01A8, 
                0x0058, 0x0048, 0x0078, 0x0068, 0x0018, 0x0008, 0x0038, 0x0028, 0x00D8, 0x00C8, 0x00F8, 0x00E8, 0x0098, 0x0088, 0x00B8, 0x00A8, 
                0x0560, 0x0520, 0x05E0, 0x05A0, 0x0460, 0x0420, 0x04E0, 0x04A0, 0x0760, 0x0720, 0x07E0, 0x07A0, 0x0660, 0x0620, 0x06E0, 0x06A0, 
                0x02B0, 0x0290, 0x02F0, 0x02D0, 0x0230, 0x0210, 0x0270, 0x0250, 0x03B0, 0x0390, 0x03F0, 0x03D0, 0x0330, 0x0310, 0x0370, 0x0350
            };

            if (fileSize < sampleCount*1) goto free_and_return_null;
            for (mo_uint64 i = 0; i < sampleCount; ++i) {
                pSamples[i] = table[pFileData8[i]];
            }
            pFileData8 += sampleCount*1; fileSize -= (size_t)sampleCount*1;
        } break;

        case 0x0007:    // WAVE_FORMAT_MULAW
        {
            mo_uint16 table[256] = {
                0x8284, 0x8684, 0x8A84, 0x8E84, 0x9284, 0x9684, 0x9A84, 0x9E84, 0xA284, 0xA684, 0xAA84, 0xAE84, 0xB284, 0xB684, 0xBA84, 0xBE84, 
                0xC184, 0xC384, 0xC584, 0xC784, 0xC984, 0xCB84, 0xCD84, 0xCF84, 0xD184, 0xD384, 0xD584, 0xD784, 0xD984, 0xDB84, 0xDD84, 0xDF84, 
                0xE104, 0xE204, 0xE304, 0xE404, 0xE504, 0xE604, 0xE704, 0xE804, 0xE904, 0xEA04, 0xEB04, 0xEC04, 0xED04, 0xEE04, 0xEF04, 0xF004, 
                0xF0C4, 0xF144, 0xF1C4, 0xF244, 0xF2C4, 0xF344, 0xF3C4, 0xF444, 0xF4C4, 0xF544, 0xF5C4, 0xF644, 0xF6C4, 0xF744, 0xF7C4, 0xF844, 
                0xF8A4, 0xF8E4, 0xF924, 0xF964, 0xF9A4, 0xF9E4, 0xFA24, 0xFA64, 0xFAA4, 0xFAE4, 0xFB24, 0xFB64, 0xFBA4, 0xFBE4, 0xFC24, 0xFC64, 
                0xFC94, 0xFCB4, 0xFCD4, 0xFCF4, 0xFD14, 0xFD34, 0xFD54, 0xFD74, 0xFD94, 0xFDB4, 0xFDD4, 0xFDF4, 0xFE14, 0xFE34, 0xFE54, 0xFE74, 
                0xFE8C, 0xFE9C, 0xFEAC, 0xFEBC, 0xFECC, 0xFEDC, 0xFEEC, 0xFEFC, 0xFF0C, 0xFF1C, 0xFF2C, 0xFF3C, 0xFF4C, 0xFF5C, 0xFF6C, 0xFF7C, 
                0xFF88, 0xFF90, 0xFF98, 0xFFA0, 0xFFA8, 0xFFB0, 0xFFB8, 0xFFC0, 0xFFC8, 0xFFD0, 0xFFD8, 0xFFE0, 0xFFE8, 0xFFF0, 0xFFF8, 0x0000, 
                0x7D7C, 0x797C, 0x757C, 0x717C, 0x6D7C, 0x697C, 0x657C, 0x617C, 0x5D7C, 0x597C, 0x557C, 0x517C, 0x4D7C, 0x497C, 0x457C, 0x417C, 
                0x3E7C, 0x3C7C, 0x3A7C, 0x387C, 0x367C, 0x347C, 0x327C, 0x307C, 0x2E7C, 0x2C7C, 0x2A7C, 0x287C, 0x267C, 0x247C, 0x227C, 0x207C, 
                0x1EFC, 0x1DFC, 0x1CFC, 0x1BFC, 0x1AFC, 0x19FC, 0x18FC, 0x17FC, 0x16FC, 0x15FC, 0x14FC, 0x13FC, 0x12FC, 0x11FC, 0x10FC, 0x0FFC, 
                0x0F3C, 0x0EBC, 0x0E3C, 0x0DBC, 0x0D3C, 0x0CBC, 0x0C3C, 0x0BBC, 0x0B3C, 0x0ABC, 0x0A3C, 0x09BC, 0x093C, 0x08BC, 0x083C, 0x07BC, 
                0x075C, 0x071C, 0x06DC, 0x069C, 0x065C, 0x061C, 0x05DC, 0x059C, 0x055C, 0x051C, 0x04DC, 0x049C, 0x045C, 0x041C, 0x03DC, 0x039C, 
                0x036C, 0x034C, 0x032C, 0x030C, 0x02EC, 0x02CC, 0x02AC, 0x028C, 0x026C, 0x024C, 0x022C, 0x020C, 0x01EC, 0x01CC, 0x01AC, 0x018C, 
                0x0174, 0x0164, 0x0154, 0x0144, 0x0134, 0x0124, 0x0114, 0x0104, 0x00F4, 0x00E4, 0x00D4, 0x00C4, 0x00B4, 0x00A4, 0x0094, 0x0084, 
                0x0078, 0x0070, 0x0068, 0x0060, 0x0058, 0x0050, 0x0048, 0x0040, 0x0038, 0x0030, 0x0028, 0x0020, 0x0018, 0x0010, 0x0008, 0x0000
            };

            if (fileSize < sampleCount*1) goto free_and_return_null;
            for (mo_uint64 i = 0; i < sampleCount; ++i) {
                pSamples[i] = table[pFileData8[i]];
            }
            pFileData8 += sampleCount*1; fileSize -= (size_t)sampleCount*1;
        } break;

        // Unknown or unsupported format.
        default: goto free_and_return_null;
    }

    if (pChannels) *pChannels = (unsigned int)channels;
    if (pSampleRate) *pSampleRate = (unsigned int)sampleRate;
    if (pSampleCount) *pSampleCount = sampleCount;
    return pSamples;

free_and_return_null:
    MO_WAV_FREE(pSamples);
    return NULL;

    #undef MO_WAV_MALLOC
    #undef MO_WAV_FREE
    #undef MO_WAV_COPY
    #undef MO_WAV_SEEK
    #undef MO_WAV_READ_UINT8
    #undef MO_WAV_READ_UINT16
    #undef MO_WAV_READ_UINT32
    #undef MO_WAV_READ_UINT64
    #undef MO_WAV_READ_GUID
    #undef MO_WAV_GUID_EQUAL
}

#ifdef MO_HAS_STB_VORBIS
static mo_int16* mo_sound_source_load__vorbis(const void* pFileData, size_t fileSize, unsigned int* pChannels, unsigned int* pSampleRate, mo_uint64* pSampleCount)
{
    if (pChannels != NULL) *pChannels = 0;
    if (pSampleRate != NULL) *pSampleRate = 0;
    if (pSampleCount != NULL) *pSampleCount = 0;

    int channels;
    int sampleRate;
    short* pTempSamples;
    int sampleCount = stb_vorbis_decode_memory((const unsigned char*)pFileData, (int)fileSize, &channels, &sampleRate, &pTempSamples);
    if (sampleCount == -1) {
        return NULL;
    }

    // The returned pointer is expected to be deallocated with mo_free(), however stb_vorbis uses standard malloc()/free() and has crappy
    // support for customizing it. It's a bit lame, but we'll need to make a copy of the returned buffer which uses mo_malloc() for the
    // allocation.
    mo_int16* pSamples = (mo_int16*)mo_malloc(sampleCount * sizeof(mo_int16));
    if (pSamples == NULL) {
        free(pTempSamples);
        return NULL;
    }

    mo_copy_memory(pSamples, pTempSamples, sampleCount * sizeof(mo_int16));
    free(pTempSamples);

    if (pChannels != NULL) *pChannels = (unsigned int)channels;
    if (pSampleRate != NULL) *pSampleRate = (unsigned int)sampleRate;
    if (pSampleCount != NULL) *pSampleCount = (mo_uint64)sampleCount;
    return pSamples;
}
#endif

#ifdef MO_HAS_DR_FLAC
static mo_int16* mo_sound_source_load__flac(const void* pFileData, size_t fileSize, unsigned int* pChannels, unsigned int* pSampleRate, mo_uint64* pSampleCount)
{
    if (pChannels != NULL) *pChannels = 0;
    if (pSampleRate != NULL) *pSampleRate = 0;
    if (pSampleCount != NULL) *pSampleCount = 0;

    unsigned int channels;
    unsigned int sampleRate;
    dr_uint64 sampleCount;
    dr_int32* pTempSamples = drflac_open_and_decode_memory_s32(pFileData, fileSize, &channels, &sampleRate, &sampleCount);
    if (pTempSamples == NULL) {
        return NULL;
    }

    // As with stb_vorbis, dr_flac has crappy support for customizing malloc/free, etc. The same issues apply here as stb_vorbis above.
    mo_int16* pSamples = (mo_int16*)mo_malloc((size_t)sampleCount * sizeof(mo_int16));
    if (pSamples == NULL) {
        free(pTempSamples);
        return NULL;
    }

    for (mo_uint64 iSample = 0; iSample < sampleCount; ++iSample) {
        pSamples[iSample] = (dr_int16)(pTempSamples[iSample] >> 16);
    }

    drflac_free(pTempSamples);

    if (pChannels != NULL) *pChannels = (unsigned int)channels;
    if (pSampleRate != NULL) *pSampleRate = (unsigned int)sampleRate;
    if (pSampleCount != NULL) *pSampleCount = (mo_uint64)sampleCount;
    return pSamples;
}
#endif

mo_result mo_sound_source_load(mo_context* pContext, const char* filePath, mo_sound_source** ppSource)
{
    if (ppSource == NULL) return MO_INVALID_ARGS;
	mo_zero_object(ppSource);

	if (pContext == NULL || filePath == NULL) return MO_INVALID_ARGS;

    size_t fileSize;
    void* pFileData = mo_open_and_read_file(pContext, filePath, &fileSize);
    if (pFileData == NULL) {
        return MO_DOES_NOT_EXIST;
    }

    unsigned int channels;
    unsigned int sampleRate;
    uint64_t totalSampleCount;
    mo_int16* pSampleDataS16 = NULL;
    
    // WAV.
    pSampleDataS16 = mo_sound_source_load__wav(pFileData, fileSize, &channels, &sampleRate, &totalSampleCount);
#ifdef MO_HAS_STB_VORBIS
    if (pSampleDataS16 == NULL) {
        pSampleDataS16 = mo_sound_source_load__vorbis(pFileData, fileSize, &channels, &sampleRate, &totalSampleCount);
    }
#endif
#ifdef MO_HAS_DR_FLAC
    if (pSampleDataS16 == NULL) {
        pSampleDataS16 = mo_sound_source_load__flac(pFileData, fileSize, &channels, &sampleRate, &totalSampleCount);
    }
#endif

    mo_free(pFileData);
    if (pSampleDataS16 == NULL) {
        return MO_INVALID_RESOURCE;
    }

    if (totalSampleCount > SIZE_MAX) {
        return MO_INVALID_RESOURCE; // The file's too big.
    }


    mo_result result = mo_sound_source_create(pContext, channels, sampleRate, totalSampleCount, pSampleDataS16, ppSource);
    mo_free(pSampleDataS16);

    return result;
}

void mo_sound_source_delete(mo_sound_source* pSource)
{
    if (pSource == NULL) return;
    mo_free(pSource);
}

mo_result mo_play_sound_source(mo_context* pContext, mo_sound_source* pSource, mo_uint32 group)
{
    if (pContext == NULL || pSource == NULL) return MO_INVALID_ARGS;

    mo_sound* pSound;
    mo_result result = mo_sound_create(pContext, pSource, group, &pSound);
    if (result != MO_SUCCESS) {
        return result;
    }

    pSound->flags |= MO_SOUND_FLAG_INLINED;
    mo_sound_play(pSound, MO_FALSE);

    return MO_SUCCESS;
}


void mo_sound_group_pause(mo_context* pContext, mo_uint32 group)
{
    if (pContext == NULL || group < MO_SOUND_GROUP_COUNT) return;
    pContext->soundGroups[group].flags |= MO_SOUND_GROUP_FLAG_PAUSED;
}

void mo_sound_group_resume(mo_context* pContext, mo_uint32 group)
{
    if (pContext == NULL || group < MO_SOUND_GROUP_COUNT) return;
    pContext->soundGroups[group].flags &= ~MO_SOUND_GROUP_FLAG_PAUSED;
}

mo_bool32 mo_sound_group_is_paused(mo_context* pContext, mo_uint32 group)
{
    if (pContext == NULL || group < MO_SOUND_GROUP_COUNT) return MO_FALSE;
    return (pContext->soundGroups[group].flags & MO_SOUND_GROUP_FLAG_PAUSED) != 0;
}

void mo_sound_group_set_volume(mo_context* pContext, mo_uint32 group, float linearVolume)
{
    if (pContext == NULL || group < MO_SOUND_GROUP_COUNT) return;
    if (linearVolume < 0) linearVolume = 0;
    pContext->soundGroups[group].linearVolume = linearVolume;
}


mo_result mo_sound_create(mo_context* pContext, mo_sound_source* pSource, mo_uint32 group, mo_sound** ppSound)
{
    if (ppSound == NULL) return MO_INVALID_ARGS;
    mo_zero_object(ppSound);

    if (pContext == NULL || pSource == NULL || group >= MO_SOUND_GROUP_COUNT) return MO_INVALID_ARGS;

    mo_sound* pSound = (mo_sound*)mo_calloc(sizeof(*pSound));
    if (pSound == NULL) {
        return MO_OUT_OF_MEMORY;
    }

    pSound->pContext = pContext;
    pSound->pSource = pSource;
    pSound->group = group;
    pSound->linearVolume = 1;
    pSound->pan = 0;

    // Add the sound to the array.
    if (pContext->soundBufferSize == pContext->soundCount) {
        mo_uint32 newSoundBufferSize = (pContext->soundBufferSize == 0) ? 8 : pContext->soundBufferSize*2;
        mo_sound** ppNewSounds = (mo_sound**)mo_realloc(pContext->ppSounds, newSoundBufferSize * sizeof(*ppNewSounds));
        if (ppNewSounds == NULL) {
            mo_free(pSound);
            return MO_OUT_OF_MEMORY;
        }

        pContext->ppSounds = ppNewSounds;
        pContext->soundBufferSize = newSoundBufferSize;
    }

    mo_assert(pContext->soundBufferSize > pContext->soundCount);
    pContext->ppSounds[pContext->soundCount] = pSound;
    pContext->soundCount += 1;


    *ppSound = pSound;
    return MO_SUCCESS;
}

void mo_sound_delete(mo_sound* pSound)
{
    if (pSound == NULL) return;

    mo_context* pContext = pSound->pContext;
    mo_assert(pContext != NULL);

    for (mo_uint32 iSound = 0; iSound < pContext->soundCount; ++iSound) {
        if (pContext->ppSounds[iSound] == pSound) {
            for (; iSound < pContext->soundCount-1; ++iSound) {
                pContext->ppSounds[iSound] = pContext->ppSounds[iSound+1];
            }

            pContext->soundCount -= 1;
            break;
        }
    }

    mo_free(pSound);
}

void mo_sound_mark_for_deletion(mo_sound* pSound)
{
    if (pSound == NULL || pSound->isMarkedForDeletion) return;

    mo_context* pContext = pSound->pContext;
    mo_assert(pContext != NULL);

    pSound->isMarkedForDeletion = MO_TRUE;
    mo_sound_stop(pSound);

    pContext->isSoundMarkedForDeletion = MO_TRUE;    
}

void mo_sound_set_volume(mo_sound* pSound, float linearVolume)
{
    if (pSound == NULL) return;
    if (linearVolume < 0) linearVolume = 0;
    pSound->linearVolume = linearVolume;
}

void mo_sound_play(mo_sound* pSound, mo_bool32 loop)
{
    if (pSound == NULL) return;
    
    if (loop) {
        pSound->flags |= MO_SOUND_FLAG_LOOPING;
    } else {
        pSound->flags &= ~MO_SOUND_FLAG_LOOPING;
    }
    
    pSound->flags |= MO_SOUND_FLAG_PLAYING;
}

void mo_sound_stop(mo_sound* pSound)
{
    if (pSound == NULL) return;
    pSound->flags &= ~MO_SOUND_FLAG_PLAYING;
}

mo_bool32 mo_sound_is_playing(mo_sound* pSound)
{
    if (pSound == NULL) return MO_FALSE;
    return (pSound->flags & MO_SOUND_FLAG_PLAYING) != 0;
}

mo_bool32 mo_sound_is_looping(mo_sound* pSound)
{
    if (pSound == NULL) return MO_FALSE;
    return (pSound->flags & MO_SOUND_FLAG_LOOPING) != 0;
}


//// Input ////

static int mo_get_button_index(mo_button button)
{
    for (int i = 0; i < MO_BUTTON_COUNT; ++i) {
        if (button & (1 << i)) {
            return i;
        }
    }

    return -1;
}

void mo_bind_key_to_button(mo_context* pContext, mo_key key, mo_button button)
{
    if (pContext == NULL) return;

    int index = mo_get_button_index(button);
    if (index == -1 || index >= MO_BUTTON_COUNT) {
        return;
    }

    pContext->keymap[index] = key;
}

mo_button mo_get_key_binding(mo_context* pContext, mo_key key)
{
    if (pContext == NULL) return 0;

    for (int i = 0; i < MO_BUTTON_COUNT; ++i) {
        if (pContext->keymap[i] == key) {
            return (mo_button)(1 << i);
        }
    }

    return 0;
}

mo_bool32 mo_is_button_down(mo_context* pContext, unsigned int button)
{
	if (pContext == NULL) return MO_FALSE;
	return (pContext->buttonState & button) != 0;
}

mo_bool32 mo_was_button_pressed(mo_context* pContext, unsigned int button)
{
	if (pContext == NULL) return MO_FALSE;
	return (pContext->buttonPressState & button) != 0;
}

mo_bool32 mo_was_button_released(mo_context* pContext, unsigned int button)
{
	if (pContext == NULL) return MO_FALSE;
	return (pContext->buttonReleaseState & button) != 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
// mini_al Implementation
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAL_IMPLEMENTATION
#if MO_USE_EXTERNAL_MINI_AL
#include "../mini_al/mini_al.h"
#else
#ifdef MAL_IMPLEMENTATION
#include <assert.h>

#ifdef MAL_WIN32
#include <windows.h>
#else
#include <stdlib.h> // For malloc()/free()
#include <string.h> // For memset()
#endif

#ifdef MAL_POSIX
#include <unistd.h>
#endif

#ifdef MAL_ENABLE_ALSA
#include <stdio.h>  // Needed for sprintf() which is used for "hw:%d,%d" formatting. TODO: Remove this later.
#endif

#if !defined(MAL_64BIT) && !defined(MAL_32BIT)
#ifdef _WIN32
#ifdef _WIN64
#define MAL_64BIT
#else
#define MAL_32BIT
#endif
#endif
#endif

#if !defined(MAL_64BIT) && !defined(MAL_32BIT)
#ifdef __GNUC__
#ifdef __LP64__
#define MAL_64BIT
#else
#define MAL_32BIT
#endif
#endif
#endif

#if !defined(MAL_64BIT) && !defined(MAL_32BIT)
#include <stdint.h>
#if INTPTR_MAX == INT64_MAX
#define MAL_64BIT
#else
#define MAL_32BIT
#endif
#endif


#ifdef MAL_WIN32
    #define MAL_THREADCALL WINAPI
    typedef unsigned long mal_thread_result;
#else
    #define MAL_THREADCALL
    typedef void* mal_thread_result;
#endif
typedef mal_thread_result (MAL_THREADCALL * mal_thread_entry_proc)(void* pData);

#define MAL_STATE_UNINITIALIZED     0
#define MAL_STATE_STOPPED           1   // The device's default state after initialization.
#define MAL_STATE_STARTED           2   // The worker thread is in it's main loop waiting for the driver to request or deliver audio data.
#define MAL_STATE_STARTING          3   // Transitioning from a stopped state to started.
#define MAL_STATE_STOPPING          4   // Transitioning from a started state to stopped.

#define MAL_DEVICE_FLAG_USING_DEFAULT_BUFFER_SIZE   (1 << 0)
#define MAL_DEVICE_FLAG_USING_DEFAULT_PERIODS       (1 << 1)


// The default size of the device's buffer in milliseconds.
//
// If this is too small you may get underruns and overruns in which case you'll need to either increase
// this value or use an explicit buffer size.
#ifndef MAL_DEFAULT_BUFFER_SIZE_IN_MILLISECONDS
#define MAL_DEFAULT_BUFFER_SIZE_IN_MILLISECONDS     25
#endif

// Default periods when none is specified in mal_device_init(). More periods means more work on the CPU.
#ifndef MAL_DEFAULT_PERIODS
#define MAL_DEFAULT_PERIODS                         2
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Standard Library Stuff
//
///////////////////////////////////////////////////////////////////////////////
#ifndef mal_zero_memory
#ifdef MAL_WIN32
#define mal_zero_memory(p, sz) ZeroMemory((p), (sz))
#else
#define mal_zero_memory(p, sz) memset((p), 0, (sz))
#endif
#endif

#define mal_zero_object(p) mal_zero_memory((p), sizeof(*(p)))

#ifndef mal_copy_memory
#ifdef MAL_WIN32
#define mal_copy_memory(dst, src, sz) CopyMemory((dst), (src), (sz))
#else
#define mal_copy_memory(dst, src, sz) memcpy((dst), (src), (sz))
#endif
#endif

#ifndef mal_malloc
#ifdef MAL_WIN32
#define mal_malloc(sz) HeapAlloc(GetProcessHeap(), 0, (sz))
#else
#define mal_malloc(sz) malloc((sz))
#endif
#endif

#ifndef mal_free
#ifdef MAL_WIN32
#define mal_free(p) HeapFree(GetProcessHeap(), 0, (p))
#else
#define mal_free(p) free((p))
#endif
#endif

#ifndef mal_assert
#ifdef MAL_WIN32
#define mal_assert(condition) assert(condition)
#else
#define mal_assert(condition) assert(condition)
#endif
#endif

// Return Values:
//   0:  Success
//   22: EINVAL
//   34: ERANGE
//
// Not using symbolic constants for errors because I want to avoid #including errno.h
static int mal_strncpy_s(char* dst, size_t dstSizeInBytes, const char* src, size_t count)
{
    if (dst == 0) {
        return 22;
    }
    if (dstSizeInBytes == 0) {
        return 22;
    }
    if (src == 0) {
        dst[0] = '\0';
        return 22;
    }

    size_t maxcount = count;
    if (count == ((size_t)-1) || count >= dstSizeInBytes) {        // -1 = _TRUNCATE
        maxcount = dstSizeInBytes - 1;
    }

    size_t i;
    for (i = 0; i < maxcount && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (src[i] == '\0' || i == count || count == ((size_t)-1)) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return 34;
}

int mal_strcmp(const char* str1, const char* str2)
{
    if (str1 == str2) return  0;

    // These checks differ from the standard implementation. It's not important, but I prefer
    // it just for sanity.
    if (str1 == NULL) return -1;
    if (str2 == NULL) return  1;

    for (;;) {
        if (str1[0] == '\0') {
            break;
        }
        if (str1[0] != str2[0]) {
            break;
        }

        str1 += 1;
        str2 += 1;
    }

    return ((unsigned char*)str1)[0] - ((unsigned char*)str2)[0];
}


// Thanks to good old Bit Twiddling Hacks for this one: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static inline unsigned int mal_next_power_of_2(unsigned int x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;

    return x;
}

static inline unsigned int mal_prev_power_of_2(unsigned int x)
{
    return mal_next_power_of_2(x) >> 1;
}


///////////////////////////////////////////////////////////////////////////////
//
// Atomics
//
///////////////////////////////////////////////////////////////////////////////
#if defined(_WIN32) && defined(_MSC_VER)
#define mal_memory_barrier()            MemoryBarrier()
#define mal_atomic_exchange_32(a, b)    InterlockedExchange((LONG*)a, (LONG)b)
#define mal_atomic_exchange_64(a, b)    InterlockedExchange64((LONGLONG*)a, (LONGLONG)b)
#define mal_atomic_increment_32(a)      InterlockedIncrement((LONG*)a)
#define mal_atomic_decrement_32(a)      InterlockedDecrement((LONG*)a)
#else
#define mal_memory_barrier()            __sync_synchronize()
#define mal_atomic_exchange_32(a, b)    (void)__sync_lock_test_and_set(a, b); __sync_synchronize()
#define mal_atomic_exchange_64(a, b)    (void)__sync_lock_test_and_set(a, b); __sync_synchronize()
#define mal_atomic_increment_32(a)      __sync_add_and_fetch(a, 1)
#define mal_atomic_decrement_32(a)      __sync_sub_and_fetch(a, 1)
#endif

#ifdef MAL_64BIT
#define mal_atomic_exchange_ptr mal_atomic_exchange_64
#endif
#ifdef MAL_32BIT
#define mal_atomic_exchange_ptr mal_atomic_exchange_32
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Timing
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MAL_WIN32
static LARGE_INTEGER g_mal_TimerFrequency = {{0}};
void mal_timer_init(mal_timer* pTimer)
{
    if (g_mal_TimerFrequency.QuadPart == 0) {
        QueryPerformanceFrequency(&g_mal_TimerFrequency);
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    pTimer->counter = (uint64_t)counter.QuadPart;
}

double mal_timer_get_time_in_seconds(mal_timer* pTimer)
{
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter)) {
        return 0;
    }

    return (counter.QuadPart - pTimer->counter) / (double)g_mal_TimerFrequency.QuadPart;
}
#else
void mal_timer_init(mal_timer* pTimer)
{
    struct timespec newTime;
    clock_gettime(CLOCK_MONOTONIC, &newTime);

    pTimer->counter = (newTime.tv_sec * 1000000000) + newTime.tv_nsec;
}

double mal_timer_get_time_in_seconds(mal_timer* pTimer)
{
    struct timespec newTime;
    clock_gettime(CLOCK_MONOTONIC, &newTime);

    uint64_t newTimeCounter = (newTime.tv_sec * 1000000000) + newTime.tv_nsec;
    uint64_t oldTimeCounter = pTimer->counter;

    return (newTimeCounter - oldTimeCounter) / 1000000000.0;
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Threading
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MAL_WIN32
mal_bool32 mal_thread_create__win32(mal_thread* pThread, mal_thread_entry_proc entryProc, void* pData)
{
    *pThread = CreateThread(NULL, 0, entryProc, pData, 0, NULL);
    if (*pThread == NULL) {
        return MAL_FALSE;
    }

    return MAL_TRUE;
}

void mal_thread_wait__win32(mal_thread* pThread)
{
    WaitForSingleObject(*pThread, INFINITE);
}

void mal_sleep__win32(mal_uint32 milliseconds)
{
    Sleep((DWORD)milliseconds);
}

void mal_yield__win32()
{
    SwitchToThread();
}


mal_bool32 mal_mutex_create__win32(mal_mutex* pMutex)
{
    *pMutex = CreateEventA(NULL, FALSE, TRUE, NULL);
    if (*pMutex == NULL) {
        return MAL_FALSE;
    }

    return MAL_TRUE;
}

void mal_mutex_delete__win32(mal_mutex* pMutex)
{
    CloseHandle(*pMutex);
}

void mal_mutex_lock__win32(mal_mutex* pMutex)
{
    WaitForSingleObject(*pMutex, INFINITE);
}

void mal_mutex_unlock__win32(mal_mutex* pMutex)
{
    SetEvent(*pMutex);
}


mal_bool32 mal_event_create__win32(mal_event* pEvent)
{
    *pEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (*pEvent == NULL) {
        return MAL_FALSE;
    }

    return MAL_TRUE;
}

void mal_event_delete__win32(mal_event* pEvent)
{
    CloseHandle(*pEvent);
}

mal_bool32 mal_event_wait__win32(mal_event* pEvent)
{
    return WaitForSingleObject(*pEvent, INFINITE) == WAIT_OBJECT_0;
}

mal_bool32 mal_event_signal__win32(mal_event* pEvent)
{
    return SetEvent(*pEvent);
}
#endif


#ifdef MAL_POSIX
mal_bool32 mal_thread_create__posix(mal_thread* pThread, mal_thread_entry_proc entryProc, void* pData)
{
    return pthread_create(pThread, NULL, entryProc, pData) == 0;
}

void mal_thread_wait__posix(mal_thread* pThread)
{
    pthread_join(*pThread, NULL);
}

void mal_sleep__posix(mal_uint32 milliseconds)
{
    usleep(milliseconds * 1000);    // <-- usleep is in microseconds.
}

void mal_yield__posix()
{
    sched_yield();
}


mal_bool32 mal_mutex_create__posix(mal_mutex* pMutex)
{
    return pthread_mutex_init(pMutex, NULL) == 0;
}

void mal_mutex_delete__posix(mal_mutex* pMutex)
{
    pthread_mutex_destroy(pMutex);
}

void mal_mutex_lock__posix(mal_mutex* pMutex)
{
    pthread_mutex_lock(pMutex);
}

void mal_mutex_unlock__posix(mal_mutex* pMutex)
{
    pthread_mutex_unlock(pMutex);
}


mal_bool32 mal_event_create__posix(mal_event* pEvent)
{
    if (pthread_mutex_init(&pEvent->mutex, NULL) != 0) {
        return MAL_FALSE;
    }

    if (pthread_cond_init(&pEvent->condition, NULL) != 0) {
        return MAL_FALSE;
    }

    pEvent->value = 0;
    return MAL_TRUE;
}

void mal_event_delete__posix(mal_event* pEvent)
{
    pthread_cond_destroy(&pEvent->condition);
    pthread_mutex_destroy(&pEvent->mutex);
}

mal_bool32 mal_event_wait__posix(mal_event* pEvent)
{
    pthread_mutex_lock(&pEvent->mutex);
    {
        while (pEvent->value == 0) {
            pthread_cond_wait(&pEvent->condition, &pEvent->mutex);
        }

        pEvent->value = 0;  // Auto-reset.
    }
    pthread_mutex_unlock(&pEvent->mutex);

    return MAL_TRUE;
}

mal_bool32 mal_event_signal__posix(mal_event* pEvent)
{
    pthread_mutex_lock(&pEvent->mutex);
    {
        pEvent->value = 1;
        pthread_cond_signal(&pEvent->condition);
    }
    pthread_mutex_unlock(&pEvent->mutex);

    return MAL_TRUE;
}
#endif

mal_bool32 mal_thread_create(mal_thread* pThread, mal_thread_entry_proc entryProc, void* pData)
{
    if (pThread == NULL || entryProc == NULL) return MAL_FALSE;

#ifdef MAL_WIN32
    return mal_thread_create__win32(pThread, entryProc, pData);
#endif
#ifdef MAL_POSIX
    return mal_thread_create__posix(pThread, entryProc, pData);
#endif
}

void mal_thread_wait(mal_thread* pThread)
{
    if (pThread == NULL) return;

#ifdef MAL_WIN32
    mal_thread_wait__win32(pThread);
#endif
#ifdef MAL_POSIX
    mal_thread_wait__posix(pThread);
#endif
}

void mal_sleep(mal_uint32 milliseconds)
{
#ifdef MAL_WIN32
    mal_sleep__win32(milliseconds);
#endif
#ifdef MAL_POSIX
    mal_sleep__posix(milliseconds);
#endif
}


mal_bool32 mal_mutex_create(mal_mutex* pMutex)
{
    if (pMutex == NULL) return MAL_FALSE;

#ifdef MAL_WIN32
    return mal_mutex_create__win32(pMutex);
#endif
#ifdef MAL_POSIX
    return mal_mutex_create__posix(pMutex);
#endif
}

void mal_mutex_delete(mal_mutex* pMutex)
{
    if (pMutex == NULL) return;

#ifdef MAL_WIN32
    mal_mutex_delete__win32(pMutex);
#endif
#ifdef MAL_POSIX
    mal_mutex_delete__posix(pMutex);
#endif
}

void mal_mutex_lock(mal_mutex* pMutex)
{
    if (pMutex == NULL) return;

#ifdef MAL_WIN32
    mal_mutex_lock__win32(pMutex);
#endif
#ifdef MAL_POSIX
    mal_mutex_lock__posix(pMutex);
#endif
}

void mal_mutex_unlock(mal_mutex* pMutex)
{
    if (pMutex == NULL) return;

#ifdef MAL_WIN32
    mal_mutex_unlock__win32(pMutex);
#endif
#ifdef MAL_POSIX
    mal_mutex_unlock__posix(pMutex);
#endif
}


mal_bool32 mal_event_create(mal_event* pEvent)
{
    if (pEvent == NULL) return MAL_FALSE;

#ifdef MAL_WIN32
    return mal_event_create__win32(pEvent);
#endif
#ifdef MAL_POSIX
    return mal_event_create__posix(pEvent);
#endif
}

void mal_event_delete(mal_event* pEvent)
{
    if (pEvent == NULL) return;

#ifdef MAL_WIN32
    mal_event_delete__win32(pEvent);
#endif
#ifdef MAL_POSIX
    mal_event_delete__posix(pEvent);
#endif
}

mal_bool32 mal_event_wait(mal_event* pEvent)
{
    if (pEvent == NULL) return MAL_FALSE;

#ifdef MAL_WIN32
    return mal_event_wait__win32(pEvent);
#endif
#ifdef MAL_POSIX
    return mal_event_wait__posix(pEvent);
#endif
}

mal_bool32 mal_event_signal(mal_event* pEvent)
{
    if (pEvent == NULL) return MAL_FALSE;

#ifdef MAL_WIN32
    return mal_event_signal__win32(pEvent);
#endif
#ifdef MAL_POSIX
    return mal_event_signal__posix(pEvent);
#endif
}


// Posts a log message.
static void mal_log(mal_device* pDevice, const char* message)
{
    if (pDevice == NULL) return;

    mal_log_proc onLog = pDevice->onLog;
    if (onLog) {
        onLog(pDevice, message);
    }
}

// Posts an error. Throw a breakpoint in here if you're needing to debug. The return value is always "resultCode".
static mal_result mal_post_error(mal_device* pDevice, const char* message, mal_result resultCode)
{
    mal_log(pDevice, message);
    return resultCode;
}



// A helper function for reading sample data from the client. Returns the number of samples read from the client. Remaining samples
// are filled with silence.
static inline mal_uint32 mal_device__read_frames_from_client(mal_device* pDevice, mal_uint32 frameCount, void* pSamples)
{
    mal_assert(pDevice != NULL);
    mal_assert(frameCount > 0);
    mal_assert(pSamples != NULL);

    mal_uint32 framesRead = 0;
    mal_send_proc onSend = pDevice->onSend;
    if (onSend) {
        framesRead = onSend(pDevice, frameCount, pSamples);
    }

    mal_uint32 samplesRead = framesRead * pDevice->channels;
    mal_uint32 sampleSize = mal_get_sample_size_in_bytes(pDevice->format);
    mal_uint32 consumedBytes = samplesRead*sampleSize;
    mal_uint32 remainingBytes = ((frameCount * pDevice->channels) - samplesRead)*sampleSize;
    mal_zero_memory((mal_uint8*)pSamples + consumedBytes, remainingBytes);

    return samplesRead;
}

// A helper for sending sample data to the client.
static inline void mal_device__send_frames_to_client(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples)
{
    mal_assert(pDevice != NULL);
    mal_assert(frameCount > 0);
    mal_assert(pSamples != NULL);

    mal_recv_proc onRecv = pDevice->onRecv;
    if (onRecv) {
        onRecv(pDevice, frameCount, pSamples);
    }
}

// A helper for changing the state of the device.
static inline void mal_device__set_state(mal_device* pDevice, mal_uint32 newState)
{
    mal_atomic_exchange_32(&pDevice->state, newState);
}

// A helper for getting the state of the device.
static inline mal_uint32 mal_device__get_state(mal_device* pDevice)
{
    return pDevice->state;
}


///////////////////////////////////////////////////////////////////////////////
//
// Null Backend
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MAL_ENABLE_NULL
static mal_result mal_enumerate_devices__null(mal_device_type type, mal_uint32* pCount, mal_device_info* pInfo)
{
    mal_uint32 infoSize = *pCount;
    *pCount = 1;    // There's only one "device" each for playback and recording for the null backend.

    if (pInfo != NULL && infoSize > 0) {
        mal_zero_object(pInfo);

        if (type == mal_device_type_playback) {
            mal_strncpy_s(pInfo->name, sizeof(pInfo->name), "NULL Playback Device", (size_t)-1);
        } else {
            mal_strncpy_s(pInfo->name, sizeof(pInfo->name), "NULL Capture Device", (size_t)-1);
        }
    }

    return MAL_SUCCESS;
}

static void mal_device_uninit__null(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);
    mal_free(pDevice->null_device.pBuffer);
}

static mal_result mal_device_init__null(mal_device* pDevice, mal_device_type type, mal_device_id* pDeviceID, mal_device_config* pConfig)
{
    (void)type;
    (void)pDeviceID;

    mal_assert(pDevice != NULL);
    pDevice->api = mal_api_null;
    pDevice->bufferSizeInFrames = pConfig->bufferSizeInFrames;
    pDevice->periods = pConfig->periods;

    pDevice->null_device.pBuffer = (mal_uint8*)mal_malloc(pDevice->bufferSizeInFrames * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format));
    if (pDevice->null_device.pBuffer == NULL) {
        return MAL_OUT_OF_MEMORY;
    }

    mal_zero_memory(pDevice->null_device.pBuffer, mal_device_get_buffer_size_in_bytes(pDevice));

    return MAL_SUCCESS;
}

static mal_result mal_device__start_backend__null(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    mal_timer_init(&pDevice->null_device.timer);
    pDevice->null_device.lastProcessedFrame = 0;

    return MAL_SUCCESS;
}

static mal_result mal_device__stop_backend__null(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);
    (void)pDevice;

    return MAL_SUCCESS;
}

static mal_result mal_device__break_main_loop__null(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    pDevice->null_device.breakFromMainLoop = MAL_TRUE;
    return MAL_SUCCESS;
}

static mal_bool32 mal_device__get_current_frame__null(mal_device* pDevice, mal_uint32* pCurrentPos)
{
    mal_assert(pDevice != NULL);
    mal_assert(pCurrentPos != NULL);
    *pCurrentPos = 0;

    mal_uint64 currentFrameAbs = (mal_uint64)(mal_timer_get_time_in_seconds(&pDevice->null_device.timer) * pDevice->sampleRate) / pDevice->channels;

    *pCurrentPos = currentFrameAbs % pDevice->bufferSizeInFrames;
    return MAL_TRUE;
}

static mal_bool32 mal_device__get_available_frames__null(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    mal_uint32 currentFrame;
    if (!mal_device__get_current_frame__null(pDevice, &currentFrame)) {
        return 0;
    }

    // In a playback device the last processed frame should always be ahead of the current frame. The space between
    // the last processed and current frame (moving forward, starting from the last processed frame) is the amount
    // of space available to write.
    //
    // For a recording device it's the other way around - the last processed frame is always _behind_ the current
    // frame and the space between is the available space.
    mal_uint32 totalFrameCount = pDevice->bufferSizeInFrames;
    if (pDevice->type == mal_device_type_playback) {
        mal_uint32 committedBeg = currentFrame;
        mal_uint32 committedEnd = pDevice->null_device.lastProcessedFrame;
        if (committedEnd <= committedBeg) {
            committedEnd += totalFrameCount;    // Wrap around.
        }

        mal_uint32 committedSize = (committedEnd - committedBeg);
        mal_assert(committedSize <= totalFrameCount);

        return totalFrameCount - committedSize;
    } else {
        mal_uint32 validBeg = pDevice->null_device.lastProcessedFrame;
        mal_uint32 validEnd = currentFrame;
        if (validEnd < validBeg) {
            validEnd += totalFrameCount;        // Wrap around.
        }

        mal_uint32 validSize = (validEnd - validBeg);
        mal_assert(validSize <= totalFrameCount);

        return validSize;
    }
}

static mal_uint32 mal_device__wait_for_frames__null(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    while (!pDevice->null_device.breakFromMainLoop) {
        mal_uint32 framesAvailable = mal_device__get_available_frames__null(pDevice);
        if (framesAvailable > 0) {
            return framesAvailable;
        }

        mal_sleep(16);
    }

    // We'll get here if the loop was terminated. Just return whatever's available.
    return mal_device__get_available_frames__null(pDevice);
}

static mal_result mal_device__main_loop__null(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    pDevice->null_device.breakFromMainLoop = MAL_FALSE;
    while (!pDevice->null_device.breakFromMainLoop) {
        mal_uint32 framesAvailable = mal_device__wait_for_frames__null(pDevice);
        if (framesAvailable == 0) {
            continue;
        }

        // If it's a playback device, don't bother grabbing more data if the device is being stopped.
        if (pDevice->null_device.breakFromMainLoop && pDevice->type == mal_device_type_playback) {
            return MAL_FALSE;
        }

        mal_uint32 sampleCount = framesAvailable * pDevice->channels;
        mal_uint32 lockOffset  = pDevice->null_device.lastProcessedFrame * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);
        mal_uint32 lockSize    = sampleCount * mal_get_sample_size_in_bytes(pDevice->format);

        if (pDevice->type == mal_device_type_playback) {
            if (pDevice->null_device.breakFromMainLoop) {
                return MAL_FALSE;
            }

            mal_device__read_frames_from_client(pDevice, framesAvailable, pDevice->null_device.pBuffer + lockOffset);
        } else {
            mal_zero_memory(pDevice->null_device.pBuffer + lockOffset, lockSize);
            mal_device__send_frames_to_client(pDevice, framesAvailable, pDevice->null_device.pBuffer + lockOffset);
        }

        pDevice->null_device.lastProcessedFrame = (pDevice->null_device.lastProcessedFrame + framesAvailable) % pDevice->bufferSizeInFrames;
    }

    return MAL_SUCCESS;
}

static mal_uint32 mal_device_get_available_rewind_amount__null(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Rewinding on the null device is unimportant. Not willing to add maintenance costs for this.
    (void)pDevice;
    return 0;
}

static mal_uint32 mal_device_rewind__null(mal_device* pDevice, mal_uint32 framesToRewind)
{
    mal_assert(pDevice != NULL);
    mal_assert(framesToRewind > 0);

    // Rewinding on the null device is unimportant. Not willing to add maintenance costs for this.
    (void)pDevice;
    (void)framesToRewind;
    return 0;
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// DirectSound Backend
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MAL_ENABLE_DSOUND
#include <dsound.h>
#include <mmreg.h>  // WAVEFORMATEX

static GUID MAL_GUID_NULL                               = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
static GUID _g_mal_GUID_IID_DirectSoundNotify           = {0xb0210783, 0x89cd, 0x11d0, {0xaf, 0x08, 0x00, 0xa0, 0xc9, 0x25, 0xcd, 0x16}};
static GUID _g_mal_GUID_IID_IDirectSoundCaptureBuffer8  = {0x00990df4, 0x0dbb, 0x4872, {0x83, 0x3e, 0x6d, 0x30, 0x3e, 0x80, 0xae, 0xb6}};
static GUID _g_mal_GUID_KSDATAFORMAT_SUBTYPE_PCM        = {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
static GUID _g_mal_GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
//static GUID _g_mal_GUID_KSDATAFORMAT_SUBTYPE_ALAW       = {0x00000006, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
//static GUID _g_mal_GUID_KSDATAFORMAT_SUBTYPE_MULAW      = {0x00000007, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
#ifdef __cplusplus
static GUID g_mal_GUID_IID_DirectSoundNotify            = _g_mal_GUID_IID_DirectSoundNotify;
static GUID g_mal_GUID_IID_IDirectSoundCaptureBuffer8   = _g_mal_GUID_IID_IDirectSoundCaptureBuffer8;
#else
static GUID* g_mal_GUID_IID_DirectSoundNotify           = &_g_mal_GUID_IID_DirectSoundNotify;
static GUID* g_mal_GUID_IID_IDirectSoundCaptureBuffer8  = &_g_mal_GUID_IID_IDirectSoundCaptureBuffer8;
#endif

typedef HRESULT (WINAPI * mal_DirectSoundCreate8Proc)(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
typedef HRESULT (WINAPI * mal_DirectSoundEnumerateAProc)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
typedef HRESULT (WINAPI * mal_DirectSoundCaptureCreate8Proc)(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8 *ppDSC8, LPUNKNOWN pUnkOuter);
typedef HRESULT (WINAPI * mal_DirectSoundCaptureEnumerateAProc)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);

static HMODULE mal_open_dsound_dll()
{
    return LoadLibraryW(L"dsound.dll");
}

static void mal_close_dsound_dll(HMODULE hModule)
{
    FreeLibrary(hModule);
}


typedef struct
{
    mal_uint32 deviceCount;
    mal_uint32 infoCount;
    mal_device_info* pInfo;
} mal_device_enum_data__dsound;

static BOOL CALLBACK mal_enum_devices_callback__dsound(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
    (void)lpcstrModule;

    mal_device_enum_data__dsound* pData = (mal_device_enum_data__dsound*)lpContext;
    assert(pData != NULL);

    if (pData->pInfo != NULL) {
        if (pData->infoCount > 0) {
            mal_zero_object(pData->pInfo);
            mal_strncpy_s(pData->pInfo->name, sizeof(pData->pInfo->name), lpcstrDescription, (size_t)-1);

            if (lpGuid != NULL) {
                mal_copy_memory(pData->pInfo->id.guid, lpGuid, 16);
            } else {
                mal_zero_memory(pData->pInfo->id.guid, 16);
            }

            pData->pInfo += 1;
            pData->infoCount -= 1;
            pData->deviceCount += 1;
        }
    } else {
        pData->deviceCount += 1;
    }

    return TRUE;
}

static mal_result mal_enumerate_devices__dsound(mal_device_type type, mal_uint32* pCount, mal_device_info* pInfo)
{
    mal_uint32 infoSize = *pCount;
    *pCount = 0;

    mal_device_enum_data__dsound enumData;
    enumData.deviceCount = 0;
    enumData.infoCount = infoSize;
    enumData.pInfo = pInfo;

    HMODULE dsoundDLL = mal_open_dsound_dll();
    if (dsoundDLL == NULL) {
        return MAL_NO_BACKEND;
    }

    if (type == mal_device_type_playback) {
        mal_DirectSoundEnumerateAProc pDirectSoundEnumerateA = (mal_DirectSoundEnumerateAProc)GetProcAddress(dsoundDLL, "DirectSoundEnumerateA");
        if (pDirectSoundEnumerateA) {
            pDirectSoundEnumerateA(mal_enum_devices_callback__dsound, &enumData);
        }
    } else {
        mal_DirectSoundCaptureEnumerateAProc pDirectSoundCaptureEnumerateA = (mal_DirectSoundCaptureEnumerateAProc)GetProcAddress(dsoundDLL, "DirectSoundCaptureEnumerateA");
        if (pDirectSoundCaptureEnumerateA) {
            pDirectSoundCaptureEnumerateA(mal_enum_devices_callback__dsound, &enumData);
        }
    }


    mal_close_dsound_dll(dsoundDLL);

    *pCount = enumData.deviceCount;
    return MAL_SUCCESS;
}

static void mal_device_uninit__dsound(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    if (pDevice->dsound.hDSoundDLL != NULL) {
        if (pDevice->dsound.pNotify) {
            IDirectSoundNotify_Release((LPDIRECTSOUNDNOTIFY)pDevice->dsound.pNotify);
        }

        if (pDevice->dsound.hRewindEvent) {
            CloseHandle(pDevice->dsound.hRewindEvent);
        }
        if (pDevice->dsound.hStopEvent) {
            CloseHandle(pDevice->dsound.hStopEvent);
        }
        for (mal_uint32 i = 0; i < pDevice->periods; ++i) {
            if (pDevice->dsound.pNotifyEvents[i]) {
                CloseHandle(pDevice->dsound.pNotifyEvents[i]);
            }
        }

        if (pDevice->dsound.pCaptureBuffer) {
            IDirectSoundCaptureBuffer8_Release((LPDIRECTSOUNDBUFFER8)pDevice->dsound.pCaptureBuffer);
        }
        if (pDevice->dsound.pCapture) {
            IDirectSoundCapture_Release((LPDIRECTSOUNDCAPTURE8)pDevice->dsound.pCapture);
        }

        if (pDevice->dsound.pPlaybackBuffer) {
            IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer);
        }
        if (pDevice->dsound.pPlaybackPrimaryBuffer) {
            IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackPrimaryBuffer);
        }
        if (pDevice->dsound.pPlayback != NULL) {
            IDirectSound_Release((LPDIRECTSOUND8)pDevice->dsound.pPlayback);
        }

        mal_close_dsound_dll((HMODULE)pDevice->dsound.hDSoundDLL);
    }
}

static mal_result mal_device_init__dsound(mal_device* pDevice, mal_device_type type, mal_device_id* pDeviceID, mal_device_config* pConfig)
{
    mal_assert(pDevice != NULL);
    pDevice->api = mal_api_dsound;
    pDevice->dsound.rewindTarget = ~0UL;

    pDevice->dsound.hDSoundDLL = (mal_handle)mal_open_dsound_dll();
    if (pDevice->dsound.hDSoundDLL == NULL) {
        return MAL_NO_BACKEND;
    }

    // Check that we have a valid format.
    GUID subformat;
    switch (pConfig->format)
    {
        case mal_format_u8:
        case mal_format_s16:
        case mal_format_s24:
        case mal_format_s32:
        {
            subformat = _g_mal_GUID_KSDATAFORMAT_SUBTYPE_PCM;
        } break;

        case mal_format_f32:
        {
            subformat = _g_mal_GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
        } break;

        default:
        return MAL_FORMAT_NOT_SUPPORTED;
    }


    WAVEFORMATEXTENSIBLE wf;
    mal_zero_object(&wf);
    wf.Format.cbSize               = sizeof(wf);
    wf.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE;
    wf.Format.nChannels            = (WORD)pConfig->channels;
    wf.Format.nSamplesPerSec       = (DWORD)pConfig->sampleRate;
    wf.Format.wBitsPerSample       = (WORD)mal_get_sample_size_in_bytes(pConfig->format)*8;
    wf.Format.nBlockAlign          = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
    wf.Format.nAvgBytesPerSec      = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
    wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
    wf.dwChannelMask               = (pConfig->channels <= 2) ? 0 : ~(((DWORD)-1) << pConfig->channels);
    wf.SubFormat                   = subformat;

    DWORD bufferSizeInBytes = 0;

    // Unfortunately DirectSound uses different APIs and data structures for playback and catpure devices :(
    if (type == mal_device_type_playback) {
        mal_DirectSoundCreate8Proc pDirectSoundCreate8 = (mal_DirectSoundCreate8Proc)GetProcAddress((HMODULE)pDevice->dsound.hDSoundDLL, "DirectSoundCreate8");
        if (pDirectSoundCreate8 == NULL) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] Could not find DirectSoundCreate8().", MAL_API_NOT_FOUND);
        }

        if (FAILED(pDirectSoundCreate8((pDeviceID == NULL) ? NULL : (LPCGUID)pDeviceID->guid, (LPDIRECTSOUND8*)&pDevice->dsound.pPlayback, NULL))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] DirectSoundCreate8() failed for playback device.", MAL_DSOUND_FAILED_TO_CREATE_DEVICE);
        }

        // The cooperative level must be set before doing anything else.
        if (FAILED(IDirectSound_SetCooperativeLevel((LPDIRECTSOUND8)pDevice->dsound.pPlayback, GetForegroundWindow(), DSSCL_PRIORITY))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] IDirectSound_SetCooperateiveLevel() failed for playback device.", MAL_DSOUND_FAILED_TO_SET_COOP_LEVEL);
        }

        DSBUFFERDESC descDSPrimary;
        mal_zero_object(&descDSPrimary);
        descDSPrimary.dwSize  = sizeof(DSBUFFERDESC);
        descDSPrimary.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
        if (FAILED(IDirectSound_CreateSoundBuffer((LPDIRECTSOUND8)pDevice->dsound.pPlayback, &descDSPrimary, (LPDIRECTSOUNDBUFFER*)&pDevice->dsound.pPlaybackPrimaryBuffer, NULL))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] IDirectSound_CreateSoundBuffer() failed for playback device's primary buffer.", MAL_DSOUND_FAILED_TO_CREATE_BUFFER);
        }

        // From MSDN:
        //
        // The method succeeds even if the hardware does not support the requested format; DirectSound sets the buffer to the closest
        // supported format. To determine whether this has happened, an application can call the GetFormat method for the primary buffer
        // and compare the result with the format that was requested with the SetFormat method.
        if (FAILED(IDirectSoundBuffer_SetFormat((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackPrimaryBuffer, (WAVEFORMATEX*)&wf))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] Failed to set format of playback device's primary buffer.", MAL_FORMAT_NOT_SUPPORTED);
        }

        // Get the _actual_ properties of the buffer. This is silly API design...
        DWORD requiredSize;
        if (FAILED(IDirectSoundBuffer_GetFormat((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackPrimaryBuffer, NULL, 0, &requiredSize))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] Failed to retrieve the actual format of the playback device's primary buffer.", MAL_FORMAT_NOT_SUPPORTED);
        }

        char rawdata[1024];
        WAVEFORMATEXTENSIBLE* pActualFormat = (WAVEFORMATEXTENSIBLE*)rawdata;
        if (FAILED(IDirectSoundBuffer_GetFormat((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackPrimaryBuffer, (WAVEFORMATEX*)pActualFormat, requiredSize, NULL))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] Failed to retrieve the actual format of the playback device's primary buffer.", MAL_FORMAT_NOT_SUPPORTED);
        }

        pDevice->channels = pActualFormat->Format.nChannels;
        pDevice->sampleRate = pActualFormat->Format.nSamplesPerSec;
        bufferSizeInBytes = pDevice->bufferSizeInFrames * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);


        // Meaning of dwFlags (from MSDN):
        //
        // DSBCAPS_CTRLPOSITIONNOTIFY
        //   The buffer has position notification capability.
        //
        // DSBCAPS_GLOBALFOCUS
        //   With this flag set, an application using DirectSound can continue to play its buffers if the user switches focus to
        //   another application, even if the new application uses DirectSound.
        //
        // DSBCAPS_GETCURRENTPOSITION2
        //   In the first version of DirectSound, the play cursor was significantly ahead of the actual playing sound on emulated
        //   sound cards; it was directly behind the write cursor. Now, if the DSBCAPS_GETCURRENTPOSITION2 flag is specified, the
        //   application can get a more accurate play cursor.
        DSBUFFERDESC descDS;
        memset(&descDS, 0, sizeof(DSBUFFERDESC));
        descDS.dwSize = sizeof(DSBUFFERDESC);
        descDS.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
        descDS.dwBufferBytes = bufferSizeInBytes;
        descDS.lpwfxFormat = (WAVEFORMATEX*)&wf;
        if (FAILED(IDirectSound_CreateSoundBuffer((LPDIRECTSOUND8)pDevice->dsound.pPlayback, &descDS, (LPDIRECTSOUNDBUFFER*)&pDevice->dsound.pPlaybackBuffer, NULL))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] IDirectSound_CreateSoundBuffer() failed for playback device's secondary buffer.", MAL_DSOUND_FAILED_TO_CREATE_BUFFER);
        }

        // Notifications are set up via a DIRECTSOUNDNOTIFY object which is retrieved from the buffer.
        if (FAILED(IDirectSoundBuffer8_QueryInterface((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer, g_mal_GUID_IID_DirectSoundNotify, (void**)&pDevice->dsound.pNotify))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] IDirectSoundBuffer8_QueryInterface() failed for playback device's IDirectSoundNotify object.", MAL_DSOUND_FAILED_TO_QUERY_INTERFACE);
        }
    } else {
        // The default buffer size is treated slightly differently for DirectSound which, for some reason, seems to
        // have worse latency with capture than playback (sometimes _much_ worse).
        if (pDevice->flags & MAL_DEVICE_FLAG_USING_DEFAULT_BUFFER_SIZE) {
            pDevice->bufferSizeInFrames *= 2; // <-- Might need to fiddle with this to find a more ideal value. May even be able to just add a fixed amount rather than scaling.
        }

        mal_DirectSoundCaptureCreate8Proc pDirectSoundCaptureCreate8 = (mal_DirectSoundCaptureCreate8Proc)GetProcAddress((HMODULE)pDevice->dsound.hDSoundDLL, "DirectSoundCaptureCreate8");
        if (pDirectSoundCaptureCreate8 == NULL) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] Could not find DirectSoundCreate8().", MAL_API_NOT_FOUND);
        }

        if (FAILED(pDirectSoundCaptureCreate8((pDeviceID == NULL) ? NULL : (LPCGUID)pDeviceID->guid, (LPDIRECTSOUNDCAPTURE8*)&pDevice->dsound.pCapture, NULL))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] DirectSoundCaptureCreate8() failed for capture device.", MAL_DSOUND_FAILED_TO_CREATE_DEVICE);
        }

        bufferSizeInBytes = pDevice->bufferSizeInFrames * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);

        DSCBUFFERDESC descDS;
        mal_zero_object(&descDS);
        descDS.dwSize = sizeof(descDS);
        descDS.dwFlags = 0;
        descDS.dwBufferBytes = bufferSizeInBytes;
        descDS.lpwfxFormat = (WAVEFORMATEX*)&wf;
        LPDIRECTSOUNDCAPTUREBUFFER pDSCB_Temp;
        if (FAILED(IDirectSoundCapture_CreateCaptureBuffer((LPDIRECTSOUNDCAPTURE8)pDevice->dsound.pCapture, &descDS, &pDSCB_Temp, NULL))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] IDirectSoundCapture_CreateCaptureBuffer() failed for capture device.", MAL_DSOUND_FAILED_TO_CREATE_BUFFER);
        }

        HRESULT hr = IDirectSoundCapture_QueryInterface(pDSCB_Temp, g_mal_GUID_IID_IDirectSoundCaptureBuffer8, (LPVOID*)&pDevice->dsound.pCaptureBuffer);
        IDirectSoundCaptureBuffer_Release(pDSCB_Temp);
        if (FAILED(hr)) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] IDirectSoundCapture_QueryInterface() failed for capture device's IDirectSoundCaptureBuffer8 object.", MAL_DSOUND_FAILED_TO_QUERY_INTERFACE);
        }

        // Notifications are set up via a DIRECTSOUNDNOTIFY object which is retrieved from the buffer.
        if (FAILED(IDirectSoundCaptureBuffer8_QueryInterface((LPDIRECTSOUNDCAPTUREBUFFER)pDevice->dsound.pCaptureBuffer, g_mal_GUID_IID_DirectSoundNotify, (void**)&pDevice->dsound.pNotify))) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] IDirectSoundCaptureBuffer8_QueryInterface() failed for capture device's IDirectSoundNotify object.", MAL_DSOUND_FAILED_TO_QUERY_INTERFACE);
        }
    }

    // We need a notification for each period. The notification offset is slightly different depending on whether or not the
    // device is a playback or capture device. For a playback device we want to be notified when a period just starts playing,
    // whereas for a capture device we want to be notified when a period has just _finished_ capturing.
    mal_uint32 periodSizeInBytes = pDevice->bufferSizeInFrames / pDevice->periods;
    DSBPOSITIONNOTIFY notifyPoints[MAL_MAX_PERIODS_DSOUND];  // One notification event for each period.
    for (mal_uint32 i = 0; i < pDevice->periods; ++i) {
        pDevice->dsound.pNotifyEvents[i] = CreateEventA(NULL, FALSE, FALSE, NULL);
        if (pDevice->dsound.pNotifyEvents[i] == NULL) {
            mal_device_uninit__dsound(pDevice);
            return mal_post_error(pDevice, "[DirectSound] Failed to create event for buffer notifications.", MAL_FAILED_TO_CREATE_EVENT);
        }

        // The notification offset is in bytes.
        notifyPoints[i].dwOffset = i * periodSizeInBytes;
        notifyPoints[i].hEventNotify = pDevice->dsound.pNotifyEvents[i];
    }

    if (FAILED(IDirectSoundNotify_SetNotificationPositions((LPDIRECTSOUNDNOTIFY)pDevice->dsound.pNotify, pDevice->periods, notifyPoints))) {
        mal_device_uninit__dsound(pDevice);
        return mal_post_error(pDevice, "[DirectSound] IDirectSoundNotify_SetNotificationPositions() failed.", MAL_DSOUND_FAILED_TO_SET_NOTIFICATIONS);
    }

    // When the device is playing the worker thread will be waiting on a bunch of notification events. To return from
    // this wait state we need to signal a special event.
    pDevice->dsound.hStopEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (pDevice->dsound.hStopEvent == NULL) {
        mal_device_uninit__dsound(pDevice);
        return mal_post_error(pDevice, "[DirectSound] Failed to create event for main loop break notification.", MAL_FAILED_TO_CREATE_EVENT);
    }

    // When the device is rewound we need to signal an event to ensure the main loop can handle it ASAP.
    pDevice->dsound.hRewindEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (pDevice->dsound.hRewindEvent == NULL) {
        mal_device_uninit__dsound(pDevice);
        return mal_post_error(pDevice, "[DirectSound] Failed to create event for main loop rewind notification.", MAL_FAILED_TO_CREATE_EVENT);
    }

    return MAL_SUCCESS;
}


static mal_result mal_device__start_backend__dsound(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    if (pDevice->type == mal_device_type_playback) {
        // Before playing anything we need to grab an initial group of samples from the client.
        mal_uint32 framesToRead = pDevice->bufferSizeInFrames / pDevice->periods;
        mal_uint32 desiredLockSize = framesToRead * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);

        void* pLockPtr;
        DWORD actualLockSize;
        void* pLockPtr2;
        DWORD actualLockSize2;
        if (SUCCEEDED(IDirectSoundBuffer_Lock((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer, 0, desiredLockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
            framesToRead = actualLockSize / mal_get_sample_size_in_bytes(pDevice->format) / pDevice->channels;
            mal_device__read_frames_from_client(pDevice, framesToRead, pLockPtr);
            IDirectSoundBuffer_Unlock((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer, pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);

            pDevice->dsound.lastProcessedFrame = framesToRead;
            if (FAILED(IDirectSoundBuffer_Play((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer, 0, 0, DSBPLAY_LOOPING))) {
                return mal_post_error(pDevice, "[DirectSound] IDirectSoundBuffer_Play() failed.", MAL_FAILED_TO_START_BACKEND_DEVICE);
            }
        } else {
            return mal_post_error(pDevice, "[DirectSound] IDirectSoundBuffer_Lock() failed.", MAL_FAILED_TO_MAP_DEVICE_BUFFER);
        }
    } else {
        if (FAILED(IDirectSoundCaptureBuffer8_Start((LPDIRECTSOUNDCAPTUREBUFFER8)pDevice->dsound.pCaptureBuffer, DSCBSTART_LOOPING))) {
            return mal_post_error(pDevice, "[DirectSound] IDirectSoundCaptureBuffer8_Start() failed.", MAL_FAILED_TO_START_BACKEND_DEVICE);
        }
    }

    return MAL_SUCCESS;
}

static mal_result mal_device__stop_backend__dsound(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    if (pDevice->type == mal_device_type_playback) {
        if (FAILED(IDirectSoundBuffer_Stop((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer))) {
            return mal_post_error(pDevice, "[DirectSound] IDirectSoundBuffer_Stop() failed.", MAL_FAILED_TO_STOP_BACKEND_DEVICE);
        }

        IDirectSoundBuffer_SetCurrentPosition((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer, 0);
    } else {
        if (FAILED(IDirectSoundCaptureBuffer_Stop((LPDIRECTSOUNDCAPTUREBUFFER)pDevice->dsound.pCaptureBuffer))) {
            return mal_post_error(pDevice, "[DirectSound] IDirectSoundCaptureBuffer_Stop() failed.", MAL_FAILED_TO_STOP_BACKEND_DEVICE);
        }
    }

    return MAL_SUCCESS;
}

static mal_result mal_device__break_main_loop__dsound(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // The main loop will be waiting on a bunch of events via the WaitForMultipleObjects() API. One of those events
    // is a special event we use for forcing that function to return.
    pDevice->dsound.breakFromMainLoop = MAL_TRUE;
    SetEvent(pDevice->dsound.hStopEvent);
    return MAL_SUCCESS;
}

static mal_bool32 mal_device__get_current_frame__dsound(mal_device* pDevice, mal_uint32* pCurrentPos)
{
    mal_assert(pDevice != NULL);
    mal_assert(pCurrentPos != NULL);
    *pCurrentPos = 0;

    DWORD dwCurrentPosition;
    if (pDevice->type == mal_device_type_playback) {
        if (FAILED(IDirectSoundBuffer_GetCurrentPosition((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer, NULL, &dwCurrentPosition))) {
            return MAL_FALSE;
        }
    } else {
        if (FAILED(IDirectSoundCaptureBuffer8_GetCurrentPosition((LPDIRECTSOUNDCAPTUREBUFFER8)pDevice->dsound.pCaptureBuffer, &dwCurrentPosition, NULL))) {
            return MAL_FALSE;
        }
    }

    *pCurrentPos = (mal_uint32)dwCurrentPosition / mal_get_sample_size_in_bytes(pDevice->format) / pDevice->channels;
    return MAL_TRUE;
}

static mal_bool32 mal_device__get_available_frames__dsound(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    mal_uint32 currentFrame;
    if (!mal_device__get_current_frame__dsound(pDevice, &currentFrame)) {
        return 0;
    }

    // In a playback device the last processed frame should always be ahead of the current frame. The space between
    // the last processed and current frame (moving forward, starting from the last processed frame) is the amount
    // of space available to write.
    //
    // For a recording device it's the other way around - the last processed frame is always _behind_ the current
    // frame and the space between is the available space.
    mal_uint32 totalFrameCount = pDevice->bufferSizeInFrames;
    if (pDevice->type == mal_device_type_playback) {
        mal_uint32 committedBeg = currentFrame;
        mal_uint32 committedEnd;
        if (pDevice->dsound.rewindTarget != ~0UL) {
            // The device was just rewound.
            committedEnd = pDevice->dsound.rewindTarget;
            if (committedEnd < committedBeg) {
                //printf("REWOUND TOO FAR: %d\n", committedBeg - committedEnd);
                committedEnd = committedBeg;
            }

            pDevice->dsound.lastProcessedFrame = committedEnd;
            pDevice->dsound.rewindTarget = ~0UL;
        } else {
            committedEnd = pDevice->dsound.lastProcessedFrame;
            if (committedEnd <= committedBeg) {
                committedEnd += totalFrameCount;
            }
        }

        mal_uint32 committedSize = (committedEnd - committedBeg);
        mal_assert(committedSize <= totalFrameCount);

        return totalFrameCount - committedSize;
    } else {
        mal_uint32 validBeg = pDevice->dsound.lastProcessedFrame;
        mal_uint32 validEnd = currentFrame;
        if (validEnd < validBeg) {
            validEnd += totalFrameCount;        // Wrap around.
        }

        mal_uint32 validSize = (validEnd - validBeg);
        mal_assert(validSize <= totalFrameCount);

        return validSize;
    }
}

static mal_uint32 mal_device__wait_for_frames__dsound(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // The timeout to use for putting the thread to sleep is based on the size of the buffer and the period count.
    DWORD timeoutInMilliseconds = (pDevice->bufferSizeInFrames / (pDevice->sampleRate/1000)) / pDevice->periods;
    if (timeoutInMilliseconds < 1) {
        timeoutInMilliseconds = 1;
    }

    unsigned int eventCount = pDevice->periods + 2;
    HANDLE pEvents[MAL_MAX_PERIODS_DSOUND + 2];   // +2 for the stop and rewind event.
    mal_copy_memory(pEvents, pDevice->dsound.pNotifyEvents, sizeof(HANDLE) * pDevice->periods);
    pEvents[eventCount-2] = pDevice->dsound.hStopEvent;
    pEvents[eventCount-1] = pDevice->dsound.hRewindEvent;

    while (!pDevice->dsound.breakFromMainLoop) {
        mal_uint32 framesAvailable = mal_device__get_available_frames__dsound(pDevice);
        if (framesAvailable > 0) {
            return framesAvailable;
        }

        // If we get here it means we weren't able to find any frames. We'll just wait here for a bit.
        WaitForMultipleObjects(eventCount, pEvents, FALSE, timeoutInMilliseconds);
    }

    // We'll get here if the loop was terminated. Just return whatever's available.
    return mal_device__get_available_frames__dsound(pDevice);
}

static mal_result mal_device__main_loop__dsound(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Make sure the stop event is not signaled to ensure we don't end up immediately returning from WaitForMultipleObjects().
    ResetEvent(pDevice->dsound.hStopEvent);

    pDevice->dsound.breakFromMainLoop = MAL_FALSE;
    while (!pDevice->dsound.breakFromMainLoop) {
        mal_uint32 framesAvailable = mal_device__wait_for_frames__dsound(pDevice);
        if (framesAvailable == 0) {
            continue;
        }

        // If it's a playback device, don't bother grabbing more data if the device is being stopped.
        if (pDevice->dsound.breakFromMainLoop && pDevice->type == mal_device_type_playback) {
            return MAL_FALSE;
        }

        DWORD lockOffset = pDevice->dsound.lastProcessedFrame * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);
        DWORD lockSize   = framesAvailable * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);

        if (pDevice->type == mal_device_type_playback) {
            if (pDevice->dsound.breakFromMainLoop) {
                return MAL_FALSE;
            }

            void* pLockPtr;
            DWORD actualLockSize;
            void* pLockPtr2;
            DWORD actualLockSize2;
            if (FAILED(IDirectSoundBuffer_Lock((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer, lockOffset, lockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
                return mal_post_error(pDevice, "[DirectSound] IDirectSoundBuffer_Lock() failed.", MAL_FAILED_TO_MAP_DEVICE_BUFFER);
            }

            mal_uint32 frameCount = actualLockSize / mal_get_sample_size_in_bytes(pDevice->format) / pDevice->channels;
            mal_device__read_frames_from_client(pDevice, frameCount, pLockPtr);
            pDevice->dsound.lastProcessedFrame = (pDevice->dsound.lastProcessedFrame + frameCount) % pDevice->bufferSizeInFrames;

            IDirectSoundBuffer_Unlock((LPDIRECTSOUNDBUFFER)pDevice->dsound.pPlaybackBuffer, pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);
        } else {
            void* pLockPtr;
            DWORD actualLockSize;
            void* pLockPtr2;
            DWORD actualLockSize2;
            if (FAILED(IDirectSoundCaptureBuffer_Lock((LPDIRECTSOUNDCAPTUREBUFFER)pDevice->dsound.pCaptureBuffer, lockOffset, lockSize, &pLockPtr, &actualLockSize, &pLockPtr2, &actualLockSize2, 0))) {
                return mal_post_error(pDevice, "[DirectSound] IDirectSoundCaptureBuffer_Lock() failed.", MAL_FAILED_TO_MAP_DEVICE_BUFFER);
            }

            mal_uint32 frameCount = actualLockSize / mal_get_sample_size_in_bytes(pDevice->format) / pDevice->channels;
            mal_device__send_frames_to_client(pDevice, frameCount, pLockPtr);
            pDevice->dsound.lastProcessedFrame = (pDevice->dsound.lastProcessedFrame + frameCount) % pDevice->bufferSizeInFrames;

            IDirectSoundCaptureBuffer_Unlock((LPDIRECTSOUNDCAPTUREBUFFER)pDevice->dsound.pCaptureBuffer, pLockPtr, actualLockSize, pLockPtr2, actualLockSize2);
        }
    }

    return MAL_SUCCESS;
}

static mal_uint32 mal_device_get_available_rewind_amount__dsound(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);
    mal_assert(pDevice->type == mal_device_type_playback);

    mal_uint32 currentFrame;
    if (!mal_device__get_current_frame__dsound(pDevice, &currentFrame)) {
        return 0;   // Failed to get the current frame.
    }

    mal_uint32 committedBeg = currentFrame;
    mal_uint32 committedEnd = pDevice->dsound.lastProcessedFrame;
    if (committedEnd <= committedBeg) {
        committedEnd += pDevice->bufferSizeInFrames;    // Wrap around.
    }

    mal_uint32 padding = (pDevice->sampleRate/1000) * 1; // <-- This is used to prevent the rewind position getting too close to the playback position.
    mal_uint32 committedSize = (committedEnd - committedBeg);
    if (committedSize < padding) {
        return 0;
    }

    return committedSize - padding;
}

static mal_uint32 mal_device_rewind__dsound(mal_device* pDevice, mal_uint32 framesToRewind)
{
    mal_assert(pDevice != NULL);
    mal_assert(framesToRewind > 0);

    // Clamp the the maximum allowable rewind amount.
    mal_uint32 maxRewind = mal_device_get_available_rewind_amount__dsound(pDevice);
    if (framesToRewind > maxRewind) {
        framesToRewind = maxRewind;
    }

    mal_uint32 desiredPosition = (pDevice->dsound.lastProcessedFrame + pDevice->bufferSizeInFrames - framesToRewind) % pDevice->bufferSizeInFrames;    // Wrap around.
    mal_atomic_exchange_32(&pDevice->dsound.rewindTarget, desiredPosition);

    SetEvent(pDevice->dsound.hRewindEvent); // Make sure the main loop is woken up so it can handle the rewind ASAP.
    return framesToRewind;
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// ALSA Backend
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MAL_ENABLE_ALSA
#include <alsa/asoundlib.h>

static const char* mal_find_char(const char* str, char c, int* index)
{
    int i = 0;
    for (;;) {
        if (str[i] == '\0') {
            if (index) *index = -1;
            return NULL;
        }

        if (str[i] == c) {
            if (index) *index = i;
            return str + i;
        }

        i += 1;
    }

    // Should never get here, but treat it as though the character was not found to make me feel
    // better inside.
    if (index) *index = -1;
    return NULL;
}

// Waits for a number of frames to become available for either capture or playback. The return
// value is the number of frames available.
//
// This will return early if the main loop is broken with mal_device__break_main_loop().
static mal_uint32 mal_device__wait_for_frames__alsa(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    while (!pDevice->alsa.breakFromMainLoop) {
        snd_pcm_sframes_t framesAvailable = snd_pcm_avail((snd_pcm_t*)pDevice->alsa.pPCM);
        if (framesAvailable > 0) {
            return framesAvailable;
        }

        if (framesAvailable < 0) {
            if (framesAvailable == -EPIPE) {
                if (snd_pcm_recover((snd_pcm_t*)pDevice->alsa.pPCM, framesAvailable, MAL_TRUE) < 0) {
                    return 0;
                }

                framesAvailable = snd_pcm_avail((snd_pcm_t*)pDevice->alsa.pPCM);
                if (framesAvailable < 0) {
                    return 0;
                }
            }
        }

        const int timeoutInMilliseconds = 20;  // <-- The larger this value, the longer it'll take to stop the device!
        int waitResult = snd_pcm_wait((snd_pcm_t*)pDevice->alsa.pPCM, timeoutInMilliseconds);
        if (waitResult < 0) {
            snd_pcm_recover((snd_pcm_t*)pDevice->alsa.pPCM, waitResult, MAL_TRUE);
        }
    }

    // We'll get here if the loop was terminated. Just return whatever's available.
    snd_pcm_sframes_t framesAvailable = snd_pcm_avail((snd_pcm_t*)pDevice->alsa.pPCM);
    if (framesAvailable < 0) {
        return 0;
    }

    return framesAvailable;
}

static mal_bool32 mal_device_write__alsa(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);
    if (!mal_device_is_started(pDevice)) {
        return MAL_FALSE;
    }
    if (pDevice->alsa.breakFromMainLoop) {
        return MAL_FALSE;
    }


    if (pDevice->alsa.pIntermediaryBuffer == NULL) {
        // mmap.
        mal_uint32 framesAvailable = mal_device__wait_for_frames__alsa(pDevice);
        if (framesAvailable == 0) {
            return MAL_FALSE;
        }

        // Don't bother asking the client for more audio data if we're just stopping the device anyway.
        if (pDevice->alsa.breakFromMainLoop) {
            return MAL_FALSE;
        }

        const snd_pcm_channel_area_t* pAreas;
        snd_pcm_uframes_t mappedOffset;
        snd_pcm_uframes_t mappedFrames = framesAvailable;
        while (framesAvailable > 0) {
            int result = snd_pcm_mmap_begin((snd_pcm_t*)pDevice->alsa.pPCM, &pAreas, &mappedOffset, &mappedFrames);
            if (result < 0) {
                return MAL_FALSE;
            }

            void* pBuffer = (mal_uint8*)pAreas[0].addr + ((pAreas[0].first + (mappedOffset * pAreas[0].step)) / 8);
            mal_device__read_frames_from_client(pDevice, mappedFrames, pBuffer);

            result = snd_pcm_mmap_commit((snd_pcm_t*)pDevice->alsa.pPCM, mappedOffset, mappedFrames);
            if (result < 0 || (snd_pcm_uframes_t)result != mappedFrames) {
                snd_pcm_recover((snd_pcm_t*)pDevice->alsa.pPCM, result, MAL_TRUE);
                return MAL_FALSE;
            }

            framesAvailable -= mappedFrames;
        }
    } else {
        // readi/writei.
        while (!pDevice->alsa.breakFromMainLoop) {
            mal_uint32 framesAvailable = mal_device__wait_for_frames__alsa(pDevice);
            if (framesAvailable == 0) {
                continue;
            }

            // Don't bother asking the client for more audio data if we're just stopping the device anyway.
            if (pDevice->alsa.breakFromMainLoop) {
                return MAL_FALSE;
            }

            mal_device__read_frames_from_client(pDevice, framesAvailable, pDevice->alsa.pIntermediaryBuffer);

            snd_pcm_sframes_t framesWritten = snd_pcm_writei((snd_pcm_t*)pDevice->alsa.pPCM, pDevice->alsa.pIntermediaryBuffer, framesAvailable);
            if (framesWritten < 0) {
                if (framesWritten == -EAGAIN) {
                    continue;   // Just keep trying...
                } else if (framesWritten == -EPIPE) {
                    // Underrun. Just recover and try writing again.
                    if (snd_pcm_recover((snd_pcm_t*)pDevice->alsa.pPCM, framesWritten, MAL_TRUE) < 0) {
                        return MAL_FALSE;
                    }

                    framesWritten = snd_pcm_writei((snd_pcm_t*)pDevice->alsa.pPCM, pDevice->alsa.pIntermediaryBuffer, framesAvailable);
                    if (framesWritten < 0) {
                        return MAL_FALSE;
                    }

                    break;  // Success.
                } else {
                    return MAL_FALSE;
                }
            } else {
                break;  // Success.
            }
        }
    }

    return MAL_TRUE;
}

static mal_bool32 mal_device_read__alsa(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);
    if (!mal_device_is_started(pDevice)) {
        return MAL_FALSE;
    }
    if (pDevice->alsa.breakFromMainLoop) {
        return MAL_FALSE;
    }

    mal_uint32 framesToSend = 0;
    void* pBuffer = NULL;
    if (pDevice->alsa.pIntermediaryBuffer == NULL) {
        // mmap.
        mal_uint32 framesAvailable = mal_device__wait_for_frames__alsa(pDevice);
        if (framesAvailable == 0) {
            return MAL_FALSE;
        }

        const snd_pcm_channel_area_t* pAreas;
        snd_pcm_uframes_t mappedOffset;
        snd_pcm_uframes_t mappedFrames = framesAvailable;
        while (framesAvailable > 0) {
            int result = snd_pcm_mmap_begin((snd_pcm_t*)pDevice->alsa.pPCM, &pAreas, &mappedOffset, &mappedFrames);
            if (result < 0) {
                return MAL_FALSE;
            }

            void* pBuffer = (mal_uint8*)pAreas[0].addr + ((pAreas[0].first + (mappedOffset * pAreas[0].step)) / 8);
            mal_device__send_frames_to_client(pDevice, mappedFrames, pBuffer);

            result = snd_pcm_mmap_commit((snd_pcm_t*)pDevice->alsa.pPCM, mappedOffset, mappedFrames);
            if (result < 0 || (snd_pcm_uframes_t)result != mappedFrames) {
                snd_pcm_recover((snd_pcm_t*)pDevice->alsa.pPCM, result, MAL_TRUE);
                return MAL_FALSE;
            }

            framesAvailable -= mappedFrames;
        }
    } else {
        // readi/writei.
        snd_pcm_sframes_t framesRead = 0;
        while (!pDevice->alsa.breakFromMainLoop) {
            mal_uint32 framesAvailable = mal_device__wait_for_frames__alsa(pDevice);
            if (framesAvailable == 0) {
                continue;
            }

            framesRead = snd_pcm_readi((snd_pcm_t*)pDevice->alsa.pPCM, pDevice->alsa.pIntermediaryBuffer, framesAvailable);
            if (framesRead < 0) {
                if (framesRead == -EAGAIN) {
                    continue;   // Just keep trying...
                } else if (framesRead == -EPIPE) {
                    // Overrun. Just recover and try reading again.
                    if (snd_pcm_recover((snd_pcm_t*)pDevice->alsa.pPCM, framesRead, MAL_TRUE) < 0) {
                        return MAL_FALSE;
                    }

                    framesRead = snd_pcm_readi((snd_pcm_t*)pDevice->alsa.pPCM, pDevice->alsa.pIntermediaryBuffer, framesAvailable);
                    if (framesRead < 0) {
                        return MAL_FALSE;
                    }

                    break;  // Success.
                } else {
                    return MAL_FALSE;
                }
            } else {
                break;  // Success.
            }
        }

        framesToSend = framesRead;
        pBuffer = pDevice->alsa.pIntermediaryBuffer;
    }

    if (framesToSend > 0) {
        mal_device__send_frames_to_client(pDevice, framesToSend, pBuffer);
    }


    if (pDevice->alsa.pIntermediaryBuffer == NULL) {
        // mmap.
    } else {
        // readi/writei.
    }

    return MAL_TRUE;
}


static mal_result mal_enumerate_devices__alsa(mal_device_type type, mal_uint32* pCount, mal_device_info* pInfo)
{
    mal_uint32 infoSize = *pCount;
    *pCount = 0;

    // What I've learned about device iteration with ALSA
    // ==================================================
    //
    // The preferred method for enumerating devices is to use snd_device_name_hint() and family. The
    // reason this is preferred is because it includes user-space devices like the "default" device
    // which goes through PulseAudio. The problem, however, is that it is extremely un-user-friendly
    // because it enumerates a _lot_ of devices. On my test machine I have only a typical output device
    // for speakers/headerphones and a microphone - this results 52 devices getting enumerated!
    //
    // One way to pull this back a bit is to ignore all but "hw" devices. At initialization time we
    // can simply append "plug" to the ID string to enable software conversions.
    //
    // An alternative enumeration technique is to use snd_card_next() and family. The problem with this
    // one, which is significant, is that it does _not_ include user-space devices.
    //
    // ---
    //
    // During my testing I have discovered that snd_pcm_open() can fail on names returned by the "NAME"
    // hint returned by snd_device_name_get_hint(). To resolve this I have needed to parse the NAME
    // string and convert it to "hw:%d,%d" format.

    char** ppDeviceHints;
    if (snd_device_name_hint(-1, "pcm", (void***)&ppDeviceHints) < 0) {
        return MAL_NO_BACKEND;
    }

    char** ppNextDeviceHint = ppDeviceHints;
    while (*ppNextDeviceHint != NULL) {
        char* NAME = snd_device_name_get_hint(*ppNextDeviceHint, "NAME");
        char* DESC = snd_device_name_get_hint(*ppNextDeviceHint, "DESC");
        char* IOID = snd_device_name_get_hint(*ppNextDeviceHint, "IOID");

        if (IOID == NULL ||
            (type == mal_device_type_playback && strcmp(IOID, "Output") == 0) ||
            (type == mal_device_type_capture  && strcmp(IOID, "Input" ) == 0))
        {
            // Experiment. Skip over any non "hw" devices to try and pull back on the number
            // of enumerated devices.
            int colonPos;
            mal_find_char(NAME, ':', &colonPos);
            if (colonPos == -1 || (colonPos == 2 && (NAME[0]=='h' && NAME[1]=='w'))) {
                if (pInfo != NULL) {
                    if (infoSize > 0) {
                        mal_zero_object(pInfo);

                        // NAME is the ID.
                        mal_strncpy_s(pInfo->id.str, sizeof(pInfo->id.str), NAME ? NAME : "", (size_t)-1);

                        // NAME -> "hw:%d,%d"
                        if (colonPos != -1 && NAME != NULL) {
                            // We need to convert the NAME string to "hw:%d,%d" format.
                            char* cardStr = NAME + 3;
                            for (;;) {
                                if (cardStr[0] == '\0') {
                                    cardStr = NULL;
                                    break;
                                }
                                if (cardStr[0] == 'C' && cardStr[1] == 'A' && cardStr[2] == 'R' && cardStr[3] == 'D' && cardStr[4] == '=') {
                                    cardStr = cardStr + 5;
                                    break;
                                }

                                cardStr += 1;
                            }

                            if (cardStr != NULL) {
                                char* deviceStr = cardStr + 1;
                                for (;;) {
                                    if (deviceStr[0] == '\0') {
                                        deviceStr = NULL;
                                        break;
                                    }
                                    if (deviceStr[0] == ',') {
                                        deviceStr[0] = '\0';    // This is the comma after the "CARD=###" part.
                                    } else {
                                        if (deviceStr[0] == 'D' && deviceStr[1] == 'E' && deviceStr[2] == 'V' && deviceStr[3] == '=') {
                                            deviceStr = deviceStr + 4;
                                            break;
                                        }
                                    }

                                    deviceStr += 1;
                                }

                                if (deviceStr != NULL) {
                                    int cardIndex = snd_card_get_index(cardStr);
                                    if (cardIndex >= 0) {
                                        sprintf(pInfo->id.str, "hw:%d,%s", cardIndex, deviceStr);
                                    }
                                }
                            }
                        }


                        // DESC is the name, followed by the description on a new line.
                        int lfPos = 0;
                        mal_find_char(DESC, '\n', &lfPos);
                        mal_strncpy_s(pInfo->name, sizeof(pInfo->name), DESC ? DESC : "", (lfPos != -1) ? (size_t)lfPos : (size_t)-1);

                        pInfo += 1;
                        infoSize -= 1;
                        *pCount += 1;
                    }
                } else {
                    *pCount += 1;
                }
            }
        }

        free(NAME);
        free(DESC);
        free(IOID);
        ppNextDeviceHint += 1;
    }

    snd_device_name_free_hint((void**)ppDeviceHints);
    return MAL_SUCCESS;
}

static void mal_device_uninit__alsa(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    if ((snd_pcm_t*)pDevice->alsa.pPCM) {
        snd_pcm_close((snd_pcm_t*)pDevice->alsa.pPCM);

        if (pDevice->alsa.pIntermediaryBuffer != NULL) {
            mal_free(pDevice->alsa.pIntermediaryBuffer);
        }
    }
}

static mal_result mal_device_init__alsa(mal_device* pDevice, mal_device_type type, mal_device_id* pDeviceID, mal_device_config* pConfig)
{
    mal_assert(pDevice != NULL);
    pDevice->api = mal_api_alsa;

    snd_pcm_format_t formatALSA;
    switch (pConfig->format)
    {
        case mal_format_u8:    formatALSA = SND_PCM_FORMAT_U8;       break;
        case mal_format_s16:   formatALSA = SND_PCM_FORMAT_S16_LE;   break;
        case mal_format_s24:   formatALSA = SND_PCM_FORMAT_S24_3LE;  break;
        case mal_format_s32:   formatALSA = SND_PCM_FORMAT_S32_LE;   break;
        case mal_format_f32:   formatALSA = SND_PCM_FORMAT_FLOAT_LE; break;
        return mal_post_error(pDevice, "[ALSA] Format not supported.", MAL_FORMAT_NOT_SUPPORTED);
    }

    char deviceName[32];
    if (pDeviceID == NULL) {
        mal_strncpy_s(deviceName, sizeof(deviceName), "default", (size_t)-1);
    } else {
        // For now, convert "hw" devices to "plughw". The reason for this is that mini_al is still a
        // a quite unstable with non "plughw" devices.
        if (pDeviceID->str[0] == 'h' && pDeviceID->str[1] == 'w' && pDeviceID->str[2] == ':') {
            deviceName[0] = 'p'; deviceName[1] = 'l'; deviceName[2] = 'u'; deviceName[3] = 'g';
            mal_strncpy_s(deviceName+4, sizeof(deviceName-4), pDeviceID->str, (size_t)-1);
        } else {
            mal_strncpy_s(deviceName, sizeof(deviceName), pDeviceID->str, (size_t)-1);
        }

    }

    if (snd_pcm_open((snd_pcm_t**)&pDevice->alsa.pPCM, deviceName, (type == mal_device_type_playback) ? SND_PCM_STREAM_PLAYBACK : SND_PCM_STREAM_CAPTURE, 0) < 0) {
        if (mal_strcmp(deviceName, "default") == 0 || mal_strcmp(deviceName, "pulse") == 0) {
            // We may have failed to open the "default" or "pulse" device, in which case try falling back to "plughw:0,0".
            mal_strncpy_s(deviceName, sizeof(deviceName), "plughw:0,0", (size_t)-1);
            if (snd_pcm_open((snd_pcm_t**)&pDevice->alsa.pPCM, deviceName, (type == mal_device_type_playback) ? SND_PCM_STREAM_PLAYBACK : SND_PCM_STREAM_CAPTURE, 0) < 0) {
                mal_device_uninit__alsa(pDevice);
                return mal_post_error(pDevice, "[ALSA] snd_pcm_open() failed.", MAL_ALSA_FAILED_TO_OPEN_DEVICE);
            }
        } else {
            mal_device_uninit__alsa(pDevice);
            return mal_post_error(pDevice, "[ALSA] snd_pcm_open() failed.", MAL_ALSA_FAILED_TO_OPEN_DEVICE);
        }
    }


    snd_pcm_hw_params_t* pHWParams = NULL;
    snd_pcm_hw_params_alloca(&pHWParams);

    if (snd_pcm_hw_params_any((snd_pcm_t*)pDevice->alsa.pPCM, pHWParams) < 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] Failed to initialize hardware parameters. snd_pcm_hw_params_any() failed.", MAL_ALSA_FAILED_TO_SET_HW_PARAMS);
    }


    // Most important properties first.

    // Sample Rate
    if (snd_pcm_hw_params_set_rate_near((snd_pcm_t*)pDevice->alsa.pPCM, pHWParams, &pConfig->sampleRate, 0) < 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] Sample rate not supported. snd_pcm_hw_params_set_rate_near() failed.", MAL_FORMAT_NOT_SUPPORTED);
    }
    pDevice->sampleRate = pConfig->sampleRate;

    // Channels.
    if (snd_pcm_hw_params_set_channels_near((snd_pcm_t*)pDevice->alsa.pPCM, pHWParams, &pConfig->channels) < 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] Failed to set channel count. snd_pcm_hw_params_set_channels_near() failed.", MAL_FORMAT_NOT_SUPPORTED);
    }
    pDevice->channels = pConfig->channels;


    // Format.
    if (snd_pcm_hw_params_set_format((snd_pcm_t*)pDevice->alsa.pPCM, pHWParams, formatALSA) < 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] Format not supported. snd_pcm_hw_params_set_format() failed.", MAL_FORMAT_NOT_SUPPORTED);
    }


    // Buffer Size
    snd_pcm_uframes_t actualBufferSize = pConfig->bufferSizeInFrames;
    if (snd_pcm_hw_params_set_buffer_size_near((snd_pcm_t*)pDevice->alsa.pPCM, pHWParams, &actualBufferSize) < 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] Failed to set buffer size for device. snd_pcm_hw_params_set_buffer_size() failed.", MAL_FORMAT_NOT_SUPPORTED);
    }


    // Periods.
    int dir = 0;
    if (snd_pcm_hw_params_set_periods_near((snd_pcm_t*)pDevice->alsa.pPCM, pHWParams, &pConfig->periods, &dir) < 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] Failed to set period count. snd_pcm_hw_params_set_periods_near() failed.", MAL_FORMAT_NOT_SUPPORTED);
    }

    pDevice->bufferSizeInFrames = actualBufferSize;
    pDevice->periods = pConfig->periods;



    // MMAP Mode
    //
    // Try using interleaved MMAP access. If this fails, fall back to standard readi/writei.
    pDevice->alsa.isUsingMMap = MAL_FALSE;
#ifdef MAL_ENABLE_EXPERIMENTAL_ALSA_MMAP
    if (snd_pcm_hw_params_set_access((snd_pcm_t*)pDevice->alsa.pPCM, pHWParams, SND_PCM_ACCESS_MMAP_INTERLEAVED) == 0) {
        pDevice->alsa.isUsingMMap = MAL_TRUE;
        mal_log(pDevice, "USING MMAP\n");
    }
#endif

    if (!pDevice->alsa.isUsingMMap) {
        if (snd_pcm_hw_params_set_access((snd_pcm_t*)pDevice->alsa.pPCM, pHWParams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {;
            mal_device_uninit__alsa(pDevice);
            return mal_post_error(pDevice, "[ALSA] Failed to set access mode to neither SND_PCM_ACCESS_MMAP_INTERLEAVED nor SND_PCM_ACCESS_RW_INTERLEAVED. snd_pcm_hw_params_set_access() failed.", MAL_FORMAT_NOT_SUPPORTED);
        }
    }


    // Apply hardware parameters.
    if (snd_pcm_hw_params((snd_pcm_t*)pDevice->alsa.pPCM, pHWParams) < 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] Failed to set hardware parameters. snd_pcm_hw_params() failed.", MAL_ALSA_FAILED_TO_SET_SW_PARAMS);
    }



    snd_pcm_sw_params_t* pSWParams = NULL;
    snd_pcm_sw_params_alloca(&pSWParams);

    if (snd_pcm_sw_params_current((snd_pcm_t*)pDevice->alsa.pPCM, pSWParams) != 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] Failed to initialize software parameters. snd_pcm_sw_params_current() failed.", MAL_ALSA_FAILED_TO_SET_SW_PARAMS);
    }

    if (snd_pcm_sw_params_set_avail_min((snd_pcm_t*)pDevice->alsa.pPCM, pSWParams, (pDevice->sampleRate/1000) * 1) != 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] snd_pcm_sw_params_set_avail_min() failed.", MAL_FORMAT_NOT_SUPPORTED);
    }

    if (type == mal_device_type_playback) {
        if (snd_pcm_sw_params_set_start_threshold((snd_pcm_t*)pDevice->alsa.pPCM, pSWParams, (pDevice->sampleRate/1000) * 1) != 0) { //mal_prev_power_of_2(pDevice->bufferSizeInFrames/pDevice->periods)
            mal_device_uninit__alsa(pDevice);
            return mal_post_error(pDevice, "[ALSA] Failed to set start threshold for playback device. snd_pcm_sw_params_set_start_threshold() failed.", MAL_ALSA_FAILED_TO_SET_SW_PARAMS);
        }
    }

    if (snd_pcm_sw_params((snd_pcm_t*)pDevice->alsa.pPCM, pSWParams) != 0) {
        mal_device_uninit__alsa(pDevice);
        return mal_post_error(pDevice, "[ALSA] Failed to set software parameters. snd_pcm_sw_params() failed.", MAL_ALSA_FAILED_TO_SET_SW_PARAMS);
    }




    // If we're _not_ using mmap we need to use an intermediary buffer.
    if (!pDevice->alsa.isUsingMMap) {
        pDevice->alsa.pIntermediaryBuffer = mal_malloc(pDevice->bufferSizeInFrames * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format));
        if (pDevice->alsa.pIntermediaryBuffer == NULL) {
            mal_device_uninit__alsa(pDevice);
            return mal_post_error(pDevice, "[ALSA] Failed to set software parameters. snd_pcm_sw_params() failed.", MAL_OUT_OF_MEMORY);
        }
    }

    return MAL_SUCCESS;
}


static mal_result mal_device__start_backend__alsa(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Prepare the device first...
    snd_pcm_prepare((snd_pcm_t*)pDevice->alsa.pPCM);

    // ... and then grab an initial chunk from the client. After this is done, the device should
    // automatically start playing, since that's how we configured the software parameters.
    if (pDevice->type == mal_device_type_playback) {
        mal_device_write__alsa(pDevice);
    } else {
        snd_pcm_start((snd_pcm_t*)pDevice->alsa.pPCM);
    }

    return MAL_SUCCESS;
}

static mal_result mal_device__stop_backend__alsa(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    snd_pcm_drop((snd_pcm_t*)pDevice->alsa.pPCM);
    return MAL_SUCCESS;
}

static mal_result mal_device__break_main_loop__alsa(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Fallback. We just set a variable to tell the worker thread to terminate after handling the
    // next bunch of frames. This is a slow way of handling this.
    pDevice->alsa.breakFromMainLoop = MAL_TRUE;
    return MAL_SUCCESS;
}

static mal_result mal_device__main_loop__alsa(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    pDevice->alsa.breakFromMainLoop = MAL_FALSE;
    if (pDevice->type == mal_device_type_playback) {
        // Playback. Read from client, write to device.
        while (!pDevice->alsa.breakFromMainLoop && mal_device_write__alsa(pDevice)) {
        }
    } else {
        // Capture. Read from device, write to client.
        while (!pDevice->alsa.breakFromMainLoop && mal_device_read__alsa(pDevice)) {
        }
    }

    return MAL_SUCCESS;
}


static mal_uint32 mal_device_get_available_rewind_amount__alsa(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Haven't figured out reliable rewinding with ALSA yet...
#if 0
    mal_uint32 padding = (pDevice->sampleRate/1000) * 1; // <-- This is used to prevent the rewind position getting too close to the playback position.

    snd_pcm_sframes_t result = snd_pcm_rewindable((snd_pcm_t*)pDevice->alsa.pPCM);
    if (result < padding) {
        return 0;
    }

    return (mal_uint32)result - padding;
#else
    return 0;
#endif
}

static mal_uint32 mal_device_rewind__alsa(mal_device* pDevice, mal_uint32 framesToRewind)
{
    mal_assert(pDevice != NULL);
    mal_assert(framesToRewind > 0);

    // Haven't figured out reliable rewinding with ALSA yet...
#if 0
    // Clamp the the maximum allowable rewind amount.
    mal_uint32 maxRewind = mal_device_get_available_rewind_amount__alsa(pDevice);
    if (framesToRewind > maxRewind) {
        framesToRewind = maxRewind;
    }

    snd_pcm_sframes_t result = snd_pcm_rewind((snd_pcm_t*)pDevice->alsa.pPCM, (snd_pcm_uframes_t)framesToRewind);
    if (result < 0) {
        return 0;
    }

    return (mal_uint32)result;
#else
    return 0;
#endif
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// OpenSL|ES Backend
//
///////////////////////////////////////////////////////////////////////////////
#ifdef MAL_ENABLE_OPENSLES
#include <SLES/OpenSLES.h>
#ifdef MAL_ANDROID
#include <SLES/OpenSLES_Android.h>
#endif

mal_result mal_enumerate_devices__sles(mal_device_type type, mal_uint32* pCount, mal_device_info* pInfo)
{
    mal_uint32 infoSize = *pCount;
    *pCount = 0;

    SLObjectItf engineObj;
    SLresult resultSL = slCreateEngine(&engineObj, 0, NULL, 0, NULL, NULL);
    if (resultSL != SL_RESULT_SUCCESS) {
        return MAL_NO_BACKEND;
    }

    (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);

    // TODO: Test Me.
    //
    // This is currently untested, so for now we are just returning default devices.
#if 0
    SLuint32 pDeviceIDs[128];
    SLint32 deviceCount = sizeof(pDeviceIDs) / sizeof(pDeviceIDs[0]);

    SLAudioIODeviceCapabilitiesItf deviceCaps;
    resultSL = (*engineObj)->GetInterface(engineObj, SL_IID_AUDIOIODEVICECAPABILITIES, &deviceCaps);
    if (resultSL != SL_RESULT_SUCCESS) {
        // The interface may not be supported so just report a default device.
        (*engineObj)->Destroy(engineObj);
        goto return_default_device;
    }

    if (type == mal_device_type_playback) {
        resultSL = (*deviceCaps)->GetAvailableAudioOutputs(deviceCaps, &deviceCount, pDeviceIDs);
        if (resultSL != SL_RESULT_SUCCESS) {
            (*engineObj)->Destroy(engineObj);
            return MAL_NO_DEVICE;
        }
    } else {
        resultSL = (*deviceCaps)->GetAvailableAudioInputs(deviceCaps, &deviceCount, pDeviceIDs);
        if (resultSL != SL_RESULT_SUCCESS) {
            (*engineObj)->Destroy(engineObj);
            return MAL_NO_DEVICE;
        }
    }

    for (SLint32 iDevice = 0; iDevice < deviceCount; ++iDevice) {
        if (pInfo != NULL) {
            if (infoSize > 0) {
                mal_zero_object(pInfo);
                pInfo->id.id32 = pDeviceIDs[iDevice];

                mal_bool32 isValidDevice = MAL_TRUE;
                if (type == mal_device_type_playback) {
                    SLAudioOutputDescriptor desc;
                    resultSL = (*deviceCaps)->QueryAudioOutputCapabilities(deviceCaps, pInfo->id.id32, &desc);
                    if (resultSL != SL_RESULT_SUCCESS) {
                        isValidDevice = MAL_FALSE;
                    }

                    mal_strncpy_s(pInfo->name, sizeof(pInfo->name), (const char*)desc.pDeviceName, (size_t)-1);
                } else {
                    SLAudioInputDescriptor desc;
                    resultSL = (*deviceCaps)->QueryAudioInputCapabilities(deviceCaps, pInfo->id.id32, &desc);
                    if (resultSL != SL_RESULT_SUCCESS) {
                        isValidDevice = MAL_FALSE;
                    }

                    mal_strncpy_s(pInfo->name, sizeof(pInfo->name), (const char*)desc.deviceName, (size_t)-1);
                }

                if (isValidDevice) {
                    pInfo += 1;
                    infoSize -= 1;
                    *pCount += 1;
                }
            }
        } else {
            *pCount += 1;
        }
    }

    (*engineObj)->Destroy(engineObj);
    return MAL_SUCCESS;
#else
    (*engineObj)->Destroy(engineObj);
    goto return_default_device;
#endif

return_default_device:
    *pCount = 1;
    if (pInfo != NULL) {
        if (infoSize > 0) {
            if (type == mal_device_type_playback) {
                pInfo->id.id32 = SL_DEFAULTDEVICEID_AUDIOOUTPUT;
                mal_strncpy_s(pInfo->name, sizeof(pInfo->name), "Default Playback Device", (size_t)-1);
            } else {
                pInfo->id.id32 = SL_DEFAULTDEVICEID_AUDIOINPUT;
                mal_strncpy_s(pInfo->name, sizeof(pInfo->name), "Default Capture Device", (size_t)-1);
            }
        }
    }

    return MAL_SUCCESS;
}


// OpenSL|ES has one-per-application objects :(
static SLObjectItf g_malEngineObjectSL = NULL;
static SLEngineItf g_malEngineSL = NULL;
static mal_uint32 g_malSLESInitCounter = 0;

#define MAL_SLES_OBJ(p)         (*((SLObjectItf)(p)))
#define MAL_SLES_OUTPUTMIX(p)   (*((SLOutputMixItf)(p)))
#define MAL_SLES_PLAY(p)        (*((SLPlayItf)(p)))
#define MAL_SLES_RECORD(p)      (*((SLRecordItf)(p)))

#ifdef MAL_ANDROID
#define MAL_SLES_BUFFERQUEUE(p) (*((SLAndroidSimpleBufferQueueItf)(p)))
#else
#define MAL_SLES_BUFFERQUEUE(p) (*((SLBufferQueueItf)(p)))
#endif

#ifdef MAL_ANDROID
//static void mal_buffer_queue_callback__sles_android(SLAndroidSimpleBufferQueueItf pBufferQueue, SLuint32 eventFlags, const void* pBuffer, SLuint32 bufferSize, SLuint32 dataUsed, void* pContext)
static void mal_buffer_queue_callback__sles_android(SLAndroidSimpleBufferQueueItf pBufferQueue, void* pUserData)
{
    (void)pBufferQueue;

    // For now, only supporting Android implementations of OpenSL|ES since that's the only one I've
    // been able to test with and I currently depend on Android-specific extensions (simple buffer
    // queues).
#ifndef MAL_ANDROID
    return MAL_NO_BACKEND;
#endif

    mal_device* pDevice = (mal_device*)pUserData;
    mal_assert(pDevice != NULL);

    // For now, don't do anything unless the buffer was fully processed. From what I can tell, it looks like
    // OpenSL|ES 1.1 improves on buffer queues to the point that we could much more intelligently handle this,
    // but unfortunately it looks like Android is only supporting OpenSL|ES 1.0.1 for now :(
    if (pDevice->state != MAL_STATE_STARTED) {
        return;
    }

    size_t periodSizeInBytes = pDevice->sles.periodSizeInFrames * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);
    mal_uint8* pBuffer = pDevice->sles.pBuffer + (pDevice->sles.currentBufferIndex * periodSizeInBytes);

    if (pDevice->type == mal_device_type_playback) {
        if (pDevice->state != MAL_STATE_STARTED) {
            return;
        }

        mal_device__read_frames_from_client(pDevice, pDevice->sles.periodSizeInFrames, pBuffer);

        SLresult resultSL = MAL_SLES_BUFFERQUEUE(pDevice->sles.pBufferQueue)->Enqueue((SLAndroidSimpleBufferQueueItf)pDevice->sles.pBufferQueue, pBuffer, periodSizeInBytes);
        if (resultSL != SL_RESULT_SUCCESS) {
            return;
        }
    } else {
        mal_device__send_frames_to_client(pDevice, pDevice->sles.periodSizeInFrames, pBuffer);

        SLresult resultSL = MAL_SLES_BUFFERQUEUE(pDevice->sles.pBufferQueue)->Enqueue((SLAndroidSimpleBufferQueueItf)pDevice->sles.pBufferQueue, pBuffer, periodSizeInBytes);
        if (resultSL != SL_RESULT_SUCCESS) {
            return;
        }
    }

    pDevice->sles.currentBufferIndex = (pDevice->sles.currentBufferIndex + 1) % pDevice->periods;
}
#endif

static void mal_device_uninit__sles(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Uninit device.
    if (pDevice->type == mal_device_type_playback) {
        if (pDevice->sles.pAudioPlayerObj) MAL_SLES_OBJ(pDevice->sles.pAudioPlayerObj)->Destroy((SLObjectItf)pDevice->sles.pAudioPlayerObj);
        if (pDevice->sles.pOutputMixObj) MAL_SLES_OBJ(pDevice->sles.pOutputMixObj)->Destroy((SLObjectItf)pDevice->sles.pOutputMixObj);
    } else {
        if (pDevice->sles.pAudioRecorderObj) MAL_SLES_OBJ(pDevice->sles.pAudioRecorderObj)->Destroy((SLObjectItf)pDevice->sles.pAudioRecorderObj);
    }

    mal_free(pDevice->sles.pBuffer);
    

    // Uninit global data.
    if (g_malSLESInitCounter > 0) {
        if (mal_atomic_decrement_32(&g_malSLESInitCounter) == 0) {
            (*g_malEngineObjectSL)->Destroy(g_malEngineObjectSL);
        }
    }
}

static mal_result mal_device_init__sles(mal_device* pDevice, mal_device_type type, mal_device_id* pDeviceID, mal_device_config* pConfig)
{
    // For now, only supporting Android implementations of OpenSL|ES since that's the only one I've
    // been able to test with and I currently depend on Android-specific extensions (simple buffer
    // queues).
#ifndef MAL_ANDROID
    return MAL_NO_BACKEND;
#endif

    // Currently only supporting simple PCM formats. 32-bit floating point is not currently supported,
    // but may be emulated later on.
    if (pConfig->format == mal_format_f32) {
        return MAL_FORMAT_NOT_SUPPORTED;
    }

    // Initialize global data first if applicable.
    if (mal_atomic_increment_32(&g_malSLESInitCounter) == 1) {
        SLresult resultSL = slCreateEngine(&g_malEngineObjectSL, 0, NULL, 0, NULL, NULL);
        if (resultSL != SL_RESULT_SUCCESS) {
            mal_atomic_decrement_32(&g_malSLESInitCounter);
            return mal_post_error(pDevice, "slCreateEngine() failed.", MAL_NO_BACKEND);
        }

        (*g_malEngineObjectSL)->Realize(g_malEngineObjectSL, SL_BOOLEAN_FALSE);

        resultSL = (*g_malEngineObjectSL)->GetInterface(g_malEngineObjectSL, SL_IID_ENGINE, &g_malEngineSL);
        if (resultSL != SL_RESULT_SUCCESS) {
            (*g_malEngineObjectSL)->Destroy(g_malEngineObjectSL);
            mal_atomic_decrement_32(&g_malSLESInitCounter);
            return mal_post_error(pDevice, "Failed to retrieve SL_IID_ENGINE interface.", MAL_NO_BACKEND);
        }
    }


    // Now we can start initializing the device properly.
    mal_assert(pDevice != NULL);
    pDevice->api = mal_api_sles;
    pDevice->sles.currentBufferIndex = 0;
    pDevice->sles.periodSizeInFrames = pConfig->bufferSizeInFrames / pConfig->periods;
    pDevice->bufferSizeInFrames = pDevice->sles.periodSizeInFrames * pConfig->periods;

    SLDataLocator_AndroidSimpleBufferQueue queue;
    queue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    queue.numBuffers = pConfig->periods;

    SLDataFormat_PCM pcm;
    pcm.formatType = SL_DATAFORMAT_PCM;
    pcm.numChannels = pConfig->channels;
    pcm.samplesPerSec = pConfig->sampleRate * 1000;  // In millihertz because, you know, the people who wrote the OpenSL|ES spec thought it would be funny to be the _only_ API to do this...
    pcm.bitsPerSample = mal_get_sample_size_in_bytes(pConfig->format) * 8;
    pcm.containerSize = pcm.bitsPerSample;  // Always tightly packed for now.
    pcm.channelMask = ~((~0UL) << pConfig->channels);
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    if (type == mal_device_type_playback) {
        if ((*g_malEngineSL)->CreateOutputMix(g_malEngineSL, (SLObjectItf*)&pDevice->sles.pOutputMixObj, 0, NULL, NULL) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to create output mix.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_OBJ(pDevice->sles.pOutputMixObj)->Realize((SLObjectItf)pDevice->sles.pOutputMixObj, SL_BOOLEAN_FALSE)) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to realize output mix object.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_OBJ(pDevice->sles.pOutputMixObj)->GetInterface((SLObjectItf)pDevice->sles.pOutputMixObj, SL_IID_OUTPUTMIX, &pDevice->sles.pOutputMix) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to retrieve SL_IID_OUTPUTMIX interface.", MAL_NO_BACKEND);
        }

        // Set the output device.
        if (pDeviceID != NULL) {
            MAL_SLES_OUTPUTMIX(pDevice->sles.pOutputMix)->ReRoute((SLOutputMixItf)pDevice->sles.pOutputMix, 1, &pDeviceID->id32);
        }

        SLDataSource source;
        source.pLocator = &queue;
        source.pFormat = &pcm;

        SLDataLocator_OutputMix outmixLocator;
        outmixLocator.locatorType = SL_DATALOCATOR_OUTPUTMIX;
        outmixLocator.outputMix = (SLObjectItf)pDevice->sles.pOutputMixObj;

        SLDataSink sink;
        sink.pLocator = &outmixLocator;
        sink.pFormat = NULL;

        const SLInterfaceID itfIDs1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
        const SLboolean itfIDsRequired1[] = {SL_BOOLEAN_TRUE};
        if ((*g_malEngineSL)->CreateAudioPlayer(g_malEngineSL, (SLObjectItf*)&pDevice->sles.pAudioPlayerObj, &source, &sink, 1, itfIDs1, itfIDsRequired1) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to create audio player.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_OBJ(pDevice->sles.pAudioPlayerObj)->Realize((SLObjectItf)pDevice->sles.pAudioPlayerObj, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to realize audio player.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_OBJ(pDevice->sles.pAudioPlayerObj)->GetInterface((SLObjectItf)pDevice->sles.pAudioPlayerObj, SL_IID_PLAY, &pDevice->sles.pAudioPlayer) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to retrieve SL_IID_PLAY interface.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_OBJ(pDevice->sles.pAudioPlayerObj)->GetInterface((SLObjectItf)pDevice->sles.pAudioPlayerObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &pDevice->sles.pBufferQueue) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to retrieve SL_IID_ANDROIDSIMPLEBUFFERQUEUE interface.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_BUFFERQUEUE(pDevice->sles.pBufferQueue)->RegisterCallback((SLAndroidSimpleBufferQueueItf)pDevice->sles.pBufferQueue, mal_buffer_queue_callback__sles_android, pDevice) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to register buffer queue callback.", MAL_NO_BACKEND);
        }
    } else {
        SLDataLocator_IODevice locatorDevice;
        locatorDevice.locatorType = SL_DATALOCATOR_IODEVICE;
        locatorDevice.deviceType = SL_IODEVICE_AUDIOINPUT;
        locatorDevice.deviceID = (pDeviceID == NULL) ? SL_DEFAULTDEVICEID_AUDIOINPUT : pDeviceID->id32;
        locatorDevice.device = NULL;

        SLDataSource source;
        source.pLocator = &locatorDevice;
        source.pFormat = NULL;

        SLDataSink sink;
        sink.pLocator = &queue;
        sink.pFormat = &pcm;

        const SLInterfaceID itfIDs1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
        const SLboolean itfIDsRequired1[] = {SL_BOOLEAN_TRUE};
        if ((*g_malEngineSL)->CreateAudioRecorder(g_malEngineSL, (SLObjectItf*)&pDevice->sles.pAudioRecorderObj, &source, &sink, 1, itfIDs1, itfIDsRequired1) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to create audio recorder.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_OBJ(pDevice->sles.pAudioRecorderObj)->Realize((SLObjectItf)pDevice->sles.pAudioRecorderObj, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to realize audio recorder.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_OBJ(pDevice->sles.pAudioRecorderObj)->GetInterface((SLObjectItf)pDevice->sles.pAudioRecorderObj, SL_IID_RECORD, &pDevice->sles.pAudioRecorder) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to retrieve SL_IID_RECORD interface.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_OBJ(pDevice->sles.pAudioRecorderObj)->GetInterface((SLObjectItf)pDevice->sles.pAudioRecorderObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &pDevice->sles.pBufferQueue) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to retrieve SL_IID_ANDROIDSIMPLEBUFFERQUEUE interface.", MAL_NO_BACKEND);
        }

        if (MAL_SLES_BUFFERQUEUE(pDevice->sles.pBufferQueue)->RegisterCallback((SLAndroidSimpleBufferQueueItf)pDevice->sles.pBufferQueue, mal_buffer_queue_callback__sles_android, pDevice) != SL_RESULT_SUCCESS) {
            mal_device_uninit__sles(pDevice);
            return mal_post_error(pDevice, "Failed to register buffer queue callback.", MAL_NO_BACKEND);
        }
    }

    size_t bufferSizeInBytes = pDevice->bufferSizeInFrames * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);
    pDevice->sles.pBuffer = (mal_uint8*)mal_malloc(bufferSizeInBytes);
    if (pDevice->sles.pBuffer == NULL) {
        mal_device_uninit__sles(pDevice);
        return mal_post_error(pDevice, "Failed to allocate memory for data buffer.", MAL_OUT_OF_MEMORY);
    }

    mal_zero_memory(pDevice->sles.pBuffer, bufferSizeInBytes);

    return MAL_SUCCESS;
}

static mal_uint32 mal_device_get_available_rewind_amount__sles(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    // Not supporting rewinding in OpenSL|ES.
    (void)pDevice;
    return 0;
}

static mal_uint32 mal_device_rewind__alsa(mal_device* pDevice, mal_uint32 framesToRewind)
{
    mal_assert(pDevice != NULL);
    mal_assert(framesToRewind > 0);

    // Not supporting rewinding in OpenSL|ES.
    (void)pDevice;
    (void)framesToRewind;
    return 0;
}

static mal_result mal_device__start_backend__sles(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    if (pDevice->type == mal_device_type_playback) {
        SLresult resultSL = MAL_SLES_PLAY(pDevice->sles.pAudioPlayer)->SetPlayState((SLPlayItf)pDevice->sles.pAudioPlayer, SL_PLAYSTATE_PLAYING);
        if (resultSL != SL_RESULT_SUCCESS) {
            return MAL_FAILED_TO_START_BACKEND_DEVICE;
        }

        // We need to enqueue a buffer for each period.
        mal_device__read_frames_from_client(pDevice, pDevice->bufferSizeInFrames, pDevice->sles.pBuffer);

        size_t periodSizeInBytes = pDevice->sles.periodSizeInFrames * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);
        for (mal_uint32 iPeriod = 0; iPeriod < pDevice->periods; ++iPeriod) {
            resultSL = MAL_SLES_BUFFERQUEUE(pDevice->sles.pBufferQueue)->Enqueue((SLAndroidSimpleBufferQueueItf)pDevice->sles.pBufferQueue, pDevice->sles.pBuffer + (periodSizeInBytes * iPeriod), periodSizeInBytes);
            if (resultSL != SL_RESULT_SUCCESS) {
                MAL_SLES_PLAY(pDevice->sles.pAudioPlayer)->SetPlayState((SLPlayItf)pDevice->sles.pAudioPlayer, SL_PLAYSTATE_STOPPED);
                return MAL_FAILED_TO_START_BACKEND_DEVICE;
            }
        }
    } else {
        SLresult resultSL = MAL_SLES_RECORD(pDevice->sles.pAudioRecorder)->SetRecordState((SLRecordItf)pDevice->sles.pAudioRecorder, SL_RECORDSTATE_RECORDING);
        if (resultSL != SL_RESULT_SUCCESS) {
            return MAL_FAILED_TO_START_BACKEND_DEVICE;
        }

        size_t periodSizeInBytes = pDevice->sles.periodSizeInFrames * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);
        for (mal_uint32 iPeriod = 0; iPeriod < pDevice->periods; ++iPeriod) {
            resultSL = MAL_SLES_BUFFERQUEUE(pDevice->sles.pBufferQueue)->Enqueue((SLAndroidSimpleBufferQueueItf)pDevice->sles.pBufferQueue, pDevice->sles.pBuffer + (periodSizeInBytes * iPeriod), periodSizeInBytes);
            if (resultSL != SL_RESULT_SUCCESS) {
                MAL_SLES_RECORD(pDevice->sles.pAudioRecorder)->SetRecordState((SLRecordItf)pDevice->sles.pAudioRecorder, SL_RECORDSTATE_STOPPED);
                return MAL_FAILED_TO_START_BACKEND_DEVICE;
            }
        }
    }

    return MAL_SUCCESS;
}

static mal_result mal_device__stop_backend__sles(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    if (pDevice->type == mal_device_type_playback) {
        SLresult resultSL = MAL_SLES_PLAY(pDevice->sles.pAudioPlayer)->SetPlayState((SLPlayItf)pDevice->sles.pAudioPlayer, SL_PLAYSTATE_STOPPED);
        if (resultSL != SL_RESULT_SUCCESS) {
            return MAL_FAILED_TO_STOP_BACKEND_DEVICE;
        }
    } else {
        SLresult resultSL = MAL_SLES_RECORD(pDevice->sles.pAudioRecorder)->SetRecordState((SLRecordItf)pDevice->sles.pAudioRecorder, SL_RECORDSTATE_STOPPED);
        if (resultSL != SL_RESULT_SUCCESS) {
            return MAL_FAILED_TO_STOP_BACKEND_DEVICE;
        }
    }

    // Make sure any queued buffers are cleared.
    MAL_SLES_BUFFERQUEUE(pDevice->sles.pBufferQueue)->Clear((SLAndroidSimpleBufferQueueItf)pDevice->sles.pBufferQueue);

    // Make sure the client is aware that the device has stopped. There may be an OpenSL|ES callback for this, but I haven't found it.
    mal_device__set_state(pDevice, MAL_STATE_STOPPED);
    if (pDevice->onStop) {
        pDevice->onStop(pDevice);
    }

    return MAL_SUCCESS;
}
#endif


static mal_result mal_device__start_backend(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    mal_result result = MAL_NO_BACKEND;
#ifdef MAL_ENABLE_DSOUND
    if (pDevice->api == mal_api_dsound) {
        result = mal_device__start_backend__dsound(pDevice);
    }
#endif
#ifdef MAL_ENABLE_ALSA
    if (pDevice->api == mal_api_alsa) {
        result = mal_device__start_backend__alsa(pDevice);
    }
#endif
#ifdef MAL_ENABLE_NULL
    if (pDevice->api == mal_api_null) {
        result = mal_device__start_backend__null(pDevice);
    }
#endif

    return result;
}

static mal_result mal_device__stop_backend(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    mal_result result = MAL_NO_BACKEND;
#ifdef MAL_ENABLE_DSOUND
    if (pDevice->api == mal_api_dsound) {
        result = mal_device__stop_backend__dsound(pDevice);
    }
#endif
#ifdef MAL_ENABLE_ALSA
    if (pDevice->api == mal_api_alsa) {
        result = mal_device__stop_backend__alsa(pDevice);
    }
#endif
#ifdef MAL_ENABLE_NULL
    if (pDevice->api == mal_api_null) {
        result = mal_device__stop_backend__null(pDevice);
    }
#endif

    return result;
}

static mal_result mal_device__break_main_loop(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    mal_result result = MAL_NO_BACKEND;
#ifdef MAL_ENABLE_DSOUND
    if (pDevice->api == mal_api_dsound) {
        result = mal_device__break_main_loop__dsound(pDevice);
    }
#endif
#ifdef MAL_ENABLE_ALSA
    if (pDevice->api == mal_api_alsa) {
        result = mal_device__break_main_loop__alsa(pDevice);
    }
#endif
#ifdef MAL_ENABLE_NULL
    if (pDevice->api == mal_api_null) {
        result = mal_device__break_main_loop__null(pDevice);
    }
#endif

    return result;
}

static mal_result mal_device__main_loop(mal_device* pDevice)
{
    mal_assert(pDevice != NULL);

    mal_result result = MAL_NO_BACKEND;
#ifdef MAL_ENABLE_DSOUND
    if (pDevice->api == mal_api_dsound) {
        result = mal_device__main_loop__dsound(pDevice);
    }
#endif
#ifdef MAL_ENABLE_ALSA
    if (pDevice->api == mal_api_alsa) {
        result = mal_device__main_loop__alsa(pDevice);
    }
#endif
#ifdef MAL_ENABLE_NULL
    if (pDevice->api == mal_api_null) {
        result = mal_device__main_loop__null(pDevice);
    }
#endif

    return result;
}

mal_thread_result MAL_THREADCALL mal_worker_thread(void* pData)
{
    mal_device* pDevice = (mal_device*)pData;
    mal_assert(pDevice != NULL);

    // This is only used to prevent posting onStop() when the device is first initialized.
    mal_bool32 skipNextStopEvent = MAL_TRUE;

    for (;;) {
        // At the start of iteration the device is stopped - we must explicitly mark it as such.
        mal_device__stop_backend(pDevice);

        if (!skipNextStopEvent) {
            mal_stop_proc onStop = pDevice->onStop;
            if (onStop) {
                onStop(pDevice);
            }
        } else {
            skipNextStopEvent = MAL_FALSE;
        }


        // Let the other threads know that the device has stopped.
        mal_device__set_state(pDevice, MAL_STATE_STOPPED);
        mal_event_signal(&pDevice->stopEvent);

        // We use an event to wait for a request to wake up.
        mal_event_wait(&pDevice->wakeupEvent);

        // Default result code.
        pDevice->workResult = MAL_SUCCESS;

        // Just break if we're terminating.
        if (mal_device__get_state(pDevice) == MAL_STATE_UNINITIALIZED) {
            break;
        }


        // Getting here means we just started the device and we need to wait for the device to
        // either deliver us data (recording) or request more data (playback).
        mal_assert(mal_device__get_state(pDevice) == MAL_STATE_STARTING);

        pDevice->workResult = mal_device__start_backend(pDevice);
        if (pDevice->workResult != MAL_SUCCESS) {
            mal_event_signal(&pDevice->startEvent);
            continue;
        }

        // The thread that requested the device to start playing is waiting for this thread to start the
        // device for real, which is now.
        mal_device__set_state(pDevice, MAL_STATE_STARTED);
        mal_event_signal(&pDevice->startEvent);

        // Now we just enter the main loop. The main loop can be broken with mal_device__break_main_loop().
        mal_device__main_loop(pDevice);
    }

    // Make sure we aren't continuously waiting on a stop event.
    mal_event_signal(&pDevice->stopEvent);  // <-- Is this still needed?
    return (mal_thread_result)0;
}


// Helper for determining whether or not the given device is initialized.
mal_bool32 mal_device__is_initialized(mal_device* pDevice)
{
    if (pDevice == NULL) return MAL_FALSE;
    return mal_device__get_state(pDevice) != MAL_STATE_UNINITIALIZED;
}


mal_result mal_enumerate_devices(mal_device_type type, mal_uint32* pCount, mal_device_info* pInfo)
{
    if (pCount == NULL) return mal_post_error(NULL, "mal_enumerate_devices() called with invalid arguments.", MAL_INVALID_ARGS);

    mal_result result = MAL_NO_BACKEND;
#ifdef MAL_ENABLE_DSOUND
    if (result != MAL_SUCCESS) {
        result = mal_enumerate_devices__dsound(type, pCount, pInfo);
    }
#endif
#ifdef MAL_ENABLE_ALSA
    if (result != MAL_SUCCESS) {
        result = mal_enumerate_devices__alsa(type, pCount, pInfo);
    }
#endif
#ifdef MAL_ENABLE_OPENSLES
    if (result != MAL_SUCCESS) {
        result = mal_enumerate_devices__sles(type, pCount, pInfo);
    }
#endif
#ifdef MAL_ENABLE_NULL
    if (result != MAL_SUCCESS) {
        result = mal_enumerate_devices__null(type, pCount, pInfo);
    }
#endif

    return result;
}

mal_result mal_device_init(mal_device* pDevice, mal_device_type type, mal_device_id* pDeviceID, mal_device_config* pConfig, void* pUserData)
{
    if (pDevice == NULL) return mal_post_error(pDevice, "mal_device_init() called with invalid arguments.", MAL_INVALID_ARGS);
    mal_zero_object(pDevice);

    // Set the user data and log callback ASAP to ensure it is available for the entire initialization process.
    pDevice->pUserData = pUserData;
    pDevice->onLog  = pConfig->onLogCallback;
    pDevice->onStop = pConfig->onStopCallback;
    pDevice->onSend = pConfig->onSendCallback;
    pDevice->onRecv = pConfig->onRecvCallback;

    if (((mal_uint64)pDevice % sizeof(pDevice)) != 0) {
        if (pDevice->onLog) {
            pDevice->onLog(pDevice, "WARNING: mal_device_init() called for a device that is not properly aligned. Thread safety is not supported.");
        }
    }

    if (pConfig == NULL || pConfig->channels == 0 || pConfig->sampleRate == 0) return mal_post_error(pDevice, "mal_device_init() called with invalid arguments.", MAL_INVALID_ARGS);

    // Default buffer size and periods.
    if (pConfig->bufferSizeInFrames == 0) {
        pConfig->bufferSizeInFrames = (pConfig->sampleRate/1000) * MAL_DEFAULT_BUFFER_SIZE_IN_MILLISECONDS;
        pDevice->flags |= MAL_DEVICE_FLAG_USING_DEFAULT_BUFFER_SIZE;
    }
    if (pConfig->periods == 0) {
        pConfig->periods = MAL_DEFAULT_PERIODS;
        pDevice->flags |= MAL_DEVICE_FLAG_USING_DEFAULT_PERIODS;
    }

    pDevice->type = type;
    pDevice->format = pConfig->format;
    pDevice->channels = pConfig->channels;
    pDevice->sampleRate = pConfig->sampleRate;
    pDevice->bufferSizeInFrames = pConfig->bufferSizeInFrames;
    pDevice->periods = pConfig->periods;

    if (!mal_mutex_create(&pDevice->lock)) {
        return mal_post_error(pDevice, "Failed to create mutex.", MAL_FAILED_TO_CREATE_MUTEX);
    }

    // When the device is started, the worker thread is the one that does the actual startup of the backend device. We
    // use a semaphore to wait for the background thread to finish the work. The same applies for stopping the device.
    //
    // Each of these semaphores is released internally by the worker thread when the work is completed. The start
    // semaphore is also used to wake up the worker thread.
    if (!mal_event_create(&pDevice->wakeupEvent)) {
        mal_mutex_delete(&pDevice->lock);
        return mal_post_error(pDevice, "Failed to create worker thread wakeup event.", MAL_FAILED_TO_CREATE_EVENT);
    }
    if (!mal_event_create(&pDevice->startEvent)) {
        mal_event_delete(&pDevice->wakeupEvent);
        mal_mutex_delete(&pDevice->lock);
        return mal_post_error(pDevice, "Failed to create worker thread start event.", MAL_FAILED_TO_CREATE_EVENT);
    }
    if (!mal_event_create(&pDevice->stopEvent)) {
        mal_event_delete(&pDevice->startEvent);
        mal_event_delete(&pDevice->wakeupEvent);
        mal_mutex_delete(&pDevice->lock);
        return mal_post_error(pDevice, "Failed to create worker thread stop event.", MAL_FAILED_TO_CREATE_EVENT);
    }


    mal_result result = MAL_NO_BACKEND;
#ifdef MAL_ENABLE_DSOUND
    if (result != MAL_SUCCESS) {
        result = mal_device_init__dsound(pDevice, type, pDeviceID, pConfig);
    }
#endif
#ifdef MAL_ENABLE_ALSA
    if (result != MAL_SUCCESS) {
        result = mal_device_init__alsa(pDevice, type, pDeviceID, pConfig);
    }
#endif
#ifdef MAL_ENABLE_OPENSLES
    if (result != MAL_SUCCESS) {
        result = mal_device_init__sles(pDevice, type, pDeviceID, pConfig);
    }
#endif
#ifdef MAL_ENABLE_NULL
    if (result != MAL_SUCCESS) {
        result = mal_device_init__null(pDevice, type, pDeviceID, pConfig);
    }
#endif

    if (result != MAL_SUCCESS) {
        return MAL_NO_BACKEND;  // The error message will have been posted by the source of the error.
    }


    // Some backends don't require the worker thread.
    if (pDevice->api != mal_api_sles) {
        // The worker thread.
        if (!mal_thread_create(&pDevice->thread, mal_worker_thread, pDevice)) {
            mal_device_uninit(pDevice);
            return mal_post_error(pDevice, "Failed to create worker thread.", MAL_FAILED_TO_CREATE_THREAD);
        }

        // Wait for the worker thread to put the device into it's stopped state for real.
        mal_event_wait(&pDevice->stopEvent);
    } else {
        mal_device__set_state(pDevice, MAL_STATE_STOPPED);
    }

    mal_assert(mal_device__get_state(pDevice) == MAL_STATE_STOPPED);
    return MAL_SUCCESS;
}

void mal_device_uninit(mal_device* pDevice)
{
    if (!mal_device__is_initialized(pDevice)) return;

    // Make sure the device is stopped first. The backends will probably handle this naturally,
    // but I like to do it explicitly for my own sanity.
    if (mal_device_is_started(pDevice)) {
        while (mal_device_stop(pDevice) == MAL_DEVICE_BUSY) {
            mal_sleep(1);
        }
    }

    // Putting the device into an uninitialized state will make the worker thread return.
    mal_device__set_state(pDevice, MAL_STATE_UNINITIALIZED);

    // Wake up the worker thread and wait for it to properly terminate.
    if (pDevice->api != mal_api_sles) {
        mal_event_signal(&pDevice->wakeupEvent);
        mal_thread_wait(&pDevice->thread);
    }

    mal_event_delete(&pDevice->stopEvent);
    mal_event_delete(&pDevice->startEvent);
    mal_event_delete(&pDevice->wakeupEvent);
    mal_mutex_delete(&pDevice->lock);

#ifdef MAL_ENABLE_DSOUND
    if (pDevice->api == mal_api_dsound) {
        mal_device_uninit__dsound(pDevice);
    }
#endif
#ifdef MAL_ENABLE_ALSA
    if (pDevice->api == mal_api_alsa) {
        mal_device_uninit__alsa(pDevice);
    }
#endif
#ifdef MAL_ENABLE_OPENSLES
    if (pDevice->api == mal_api_sles) {
        mal_device_uninit__sles(pDevice);
    }
#endif
#ifdef MAL_ENABLE_NULL
    if (pDevice->api == mal_api_null) {
        mal_device_uninit__null(pDevice);
    }
#endif

    mal_zero_object(pDevice);
}

void mal_device_set_recv_callback(mal_device* pDevice, mal_recv_proc proc)
{
    if (pDevice == NULL) return;
    mal_atomic_exchange_ptr(&pDevice->onRecv, proc);
}

void mal_device_set_send_callback(mal_device* pDevice, mal_send_proc proc)
{
    if (pDevice == NULL) return;
    mal_atomic_exchange_ptr(&pDevice->onSend, proc);
}

void mal_device_set_stop_callback(mal_device* pDevice, mal_stop_proc proc)
{
    if (pDevice == NULL) return;
    mal_atomic_exchange_ptr(&pDevice->onStop, proc);
}

mal_result mal_device_start(mal_device* pDevice)
{
    if (pDevice == NULL) return mal_post_error(pDevice, "mal_device_start() called with invalid arguments.", MAL_INVALID_ARGS);
    if (mal_device__get_state(pDevice) == MAL_STATE_UNINITIALIZED) return mal_post_error(pDevice, "mal_device_start() called for an uninitialized device.", MAL_DEVICE_NOT_INITIALIZED);

    mal_result result = MAL_ERROR;
    mal_mutex_lock(&pDevice->lock);
    {
        // Be a bit more descriptive if the device is already started or is already in the process of starting. This is likely
        // a bug with the application.
        if (mal_device__get_state(pDevice) == MAL_STATE_STARTING) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, "mal_device_start() called while another thread is already starting it.", MAL_DEVICE_ALREADY_STARTING);
        }
        if (mal_device__get_state(pDevice) == MAL_STATE_STARTED) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, "mal_device_start() called for a device that's already started.", MAL_DEVICE_ALREADY_STARTED);
        }

        // The device needs to be in a stopped state. If it's not, we just let the caller know the device is busy.
        if (mal_device__get_state(pDevice) != MAL_STATE_STOPPED) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, "mal_device_start() called while another thread is in the process of stopping it.", MAL_DEVICE_BUSY);
        }

        mal_device__set_state(pDevice, MAL_STATE_STARTING);

        // Asynchronous backends need to be handled differently.
#ifdef MAL_ENABLE_OPENSLES
        if (pDevice->api == mal_api_sles) {
            mal_device__start_backend__sles(pDevice);
            mal_device__set_state(pDevice, MAL_STATE_STARTED);
        } else
#endif
        // Synchronous backends.
        {
            mal_event_signal(&pDevice->wakeupEvent);

            // Wait for the worker thread to finish starting the device. Note that the worker thread will be the one
            // who puts the device into the started state. Don't call mal_device__set_state() here.
            mal_event_wait(&pDevice->startEvent);
            result = pDevice->workResult;
        }
    }
    mal_mutex_unlock(&pDevice->lock);

    return result;
}

mal_result mal_device_stop(mal_device* pDevice)
{
    if (pDevice == NULL) return mal_post_error(pDevice, "mal_device_stop() called with invalid arguments.", MAL_INVALID_ARGS);
    if (mal_device__get_state(pDevice) == MAL_STATE_UNINITIALIZED) return mal_post_error(pDevice, "mal_device_stop() called for an uninitialized device.", MAL_DEVICE_NOT_INITIALIZED);

    mal_result result = MAL_ERROR;
    mal_mutex_lock(&pDevice->lock);
    {
        // Be a bit more descriptive if the device is already stopped or is already in the process of stopping. This is likely
        // a bug with the application.
        if (mal_device__get_state(pDevice) == MAL_STATE_STOPPING) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, "mal_device_stop() called while another thread is already stopping it.", MAL_DEVICE_ALREADY_STOPPING);
        }
        if (mal_device__get_state(pDevice) == MAL_STATE_STOPPED) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, "mal_device_stop() called for a device that's already stopped.", MAL_DEVICE_ALREADY_STOPPED);
        }

        // The device needs to be in a started state. If it's not, we just let the caller know the device is busy.
        if (mal_device__get_state(pDevice) != MAL_STATE_STARTED) {
            mal_mutex_unlock(&pDevice->lock);
            return mal_post_error(pDevice, "mal_device_stop() called while another thread is in the process of starting it.", MAL_DEVICE_BUSY);
        }

        mal_device__set_state(pDevice, MAL_STATE_STOPPING);

        // There's no need to wake up the thread like we do when starting.

        // Asynchronous backends need to be handled differently.
#ifdef MAL_ENABLE_OPENSLES
        if (pDevice->api == mal_api_sles) {
            mal_device__stop_backend__sles(pDevice);
        } else
#endif
        // Synchronous backends.
        {
            // When we get here the worker thread is likely in a wait state while waiting for the backend device to deliver or request
            // audio data. We need to force these to return as quickly as possible.
            mal_device__break_main_loop(pDevice);

            // We need to wait for the worker thread to become available for work before returning. Note that the worker thread will be
            // the one who puts the device into the stopped state. Don't call mal_device__set_state() here.
            mal_event_wait(&pDevice->stopEvent);
            result = MAL_SUCCESS;
        }
    }
    mal_mutex_unlock(&pDevice->lock);

    return result;
}

mal_bool32 mal_device_is_started(mal_device* pDevice)
{
    if (pDevice == NULL) return MAL_FALSE;
    return mal_device__get_state(pDevice) == MAL_STATE_STARTED;
}

mal_uint32 mal_device_get_available_rewind_amount(mal_device* pDevice)
{
    if (pDevice == NULL) return 0;

    // Only playback devices can be rewound.
    if (pDevice->type != mal_device_type_playback) {
        return 0;
    }

#ifdef MAL_ENABLE_DSOUND
    if (pDevice->api == mal_api_dsound) {
        mal_mutex_lock(&pDevice->lock);
        mal_uint32 result = mal_device_get_available_rewind_amount__dsound(pDevice);
        mal_mutex_unlock(&pDevice->lock);
        return result;
    }
#endif
#ifdef MAL_ENABLE_ALSA
    if (pDevice->api == mal_api_alsa) {
        mal_mutex_lock(&pDevice->lock);
        mal_uint32 result = mal_device_get_available_rewind_amount__alsa(pDevice);
        mal_mutex_unlock(&pDevice->lock);
        return result;
    }
#endif
#ifdef MAL_ENABLE_NULL
    if (pDevice->api == mal_api_null) {
        mal_mutex_lock(&pDevice->lock);
        mal_uint32 result = mal_device_get_available_rewind_amount__null(pDevice);
        mal_mutex_unlock(&pDevice->lock);
        return result;
    }
#endif

    return 0;
}

mal_uint32 mal_device_rewind(mal_device* pDevice, mal_uint32 framesToRewind)
{
    if (pDevice == NULL || framesToRewind == 0) return 0;

    // Only playback devices can be rewound.
    if (pDevice->type != mal_device_type_playback) {
        return 0;
    }

#ifdef MAL_ENABLE_DSOUND
    if (pDevice->api == mal_api_dsound) {
        mal_mutex_lock(&pDevice->lock);
        mal_uint32 result = mal_device_rewind__dsound(pDevice, framesToRewind);
        mal_mutex_unlock(&pDevice->lock);
        return result;
    }
#endif
#ifdef MAL_ENABLE_ALSA
    if (pDevice->api == mal_api_alsa) {
        mal_mutex_lock(&pDevice->lock);
        mal_uint32 result = mal_device_rewind__alsa(pDevice, framesToRewind);
        mal_mutex_unlock(&pDevice->lock);
        return result;
    }
#endif
#ifdef MAL_ENABLE_NULL
    if (pDevice->api == mal_api_null) {
        mal_mutex_lock(&pDevice->lock);
        mal_uint32 result = mal_device_rewind__null(pDevice, framesToRewind);
        mal_mutex_unlock(&pDevice->lock);
        return result;
    }
#endif

    return 0;
}

mal_uint32 mal_device_get_buffer_size_in_bytes(mal_device* pDevice)
{
    if (pDevice == NULL) return 0;
    return pDevice->bufferSizeInFrames * pDevice->channels * mal_get_sample_size_in_bytes(pDevice->format);
}

mal_uint32 mal_get_sample_size_in_bytes(mal_format format)
{
    mal_uint32 sizes[] = {
        1,  // u8
        2,  // s16
        3,  // s24
        4,  // s32
        4,  // f32
    };
    return sizes[format];
}
#endif  //MAL_IMPLEMENTATION
#endif  //MO_USE_EXTERNAL_MINI_AL

#endif	//MINTARO_IMPLEMENTATION


// REVISION HISTORY
// ================
//
// v0.1 - TBD
//   - Initial versioned release.


/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
