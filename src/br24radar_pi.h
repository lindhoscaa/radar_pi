/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navico BR24 Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2013 by Kees Verruijt         canboat@verruijt.net *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _BR24RADARPI_H_
#define _BR24RADARPI_H_

#include "wx/wxprec.h"
#include <wx/glcanvas.h>

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#define     PLUGIN_VERSION_MAJOR    0
#define     PLUGIN_VERSION_MINOR    41102

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    8

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

#ifdef __WXGTK__
//#include "/home/dsr/Projects/opencpn_sf/opencpn/include/ocpn_plugin.h"
#endif

#ifdef __WXMSW__
//#include "../../opencpn_sf/opencpn/include/ocpn_plugin.h"
#endif

#include "ocpn_plugin.h"
#include "nmea0183/nmea0183.h"


#ifndef SOCKET
# define SOCKET int
#endif
#ifndef INVALID_SOCKET
# define INVALID_SOCKET ((SOCKET)~0)
#endif

#ifdef __WXMSW__
# define SOCKETERRSTR (strerror(WSAGetLastError()))
#else
# include <errno.h>
# define SOCKETERRSTR (strerror(errno))
# define closesocket(fd) close(fd)
#endif

# define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))

enum {
    BM_ID_RED,
    BM_ID_RED_SLAVE,
    BM_ID_GREEN,
    BM_ID_GREEN_SLAVE,
    BM_ID_AMBER,
    BM_ID_AMBER_SLAVE,
    BM_ID_BLANK,
    BM_ID_BLANK_SLAVE

};

enum {
    RADAR_OFF,
    RADAR_ON,
};

#pragma pack(push,1)

struct br24_header {
    UINT8 headerLen;       //1 bytes
    UINT8 status;          //1 bytes
    UINT8 scan_number[2];  //2 bytes
    UINT8 mark[4];         //4 bytes 0x00, 0x44, 0x0d, 0x0e
    UINT8 angle[2];        //2 bytes
    UINT8 u00[2];          //2 bytes blank
    UINT8 range[4];        //4 bytes
    UINT8 u01[2];          //2 bytes blank
    UINT8 u02[2];          //2 bytes
    UINT8 u03[4];          //4 bytes blank
}; /* total size = 24 */

struct br4g_header {
    UINT8 headerLen;       //1 bytes
    UINT8 status;          //1 bytes
    UINT8 scan_number[2];  //2 bytes
    UINT8 u00[2];          //Always 0x4400 (integer)
    UINT8 largerange[2];   //2 bytes or -1
    UINT8 angle[2];        //2 bytes
    UINT8 u01[2];          //Always 0x8000 = -1
    UINT8 smallrange[2];   //2 bytes or -1
    UINT8 rotation[2];     //2 bytes, looks like rotation/angle
    UINT8 u02[4];          //4 bytes signed integer, always -1
    UINT8 u03[4];          //4 bytes signed integer, mostly -1 (0x80 in last byte) or 0xa0 in last byte
}; /* total size = 24 */

struct radar_line {
    union {
        br24_header   br24;
        br4g_header   br4g;
    };
    UINT8 data[512];
};


/* Normally the packets are have 32 spokes, or scan lines, but we assume nothing
 * so we take up to 120 spokes. This is the nearest round figure without going over
 * 64kB.
 */

struct radar_frame_pkt {
    UINT8      frame_hdr[8];
    radar_line line[120];    //  scan lines, or spokes
};
#pragma pack(pop)

struct receive_statistics {
    int packets;
    int broken_packets;
    int spokes;
    int broken_spokes;
    int missing_spokes;
};

static wxFont g_font;
static wxSize g_buttonSize;

typedef enum ControlType {
    CT_RANGE,
    CT_GAIN,
    CT_SEA,
    CT_RAIN,
    CT_TRANSPARENCY,
    CT_INTERFERENCE_REJECTION,
    CT_TARGET_SEPARATION,
    CT_NOISE_REJECTION,
    CT_TARGET_BOOST,
    CT_SCAN_SPEED,
    CT_SCAN_AGE
} ControlType;

typedef enum GuardZoneType {
    GZ_OFF,
    GZ_ARC,
    GZ_CIRCLE
} GuardZoneType;

extern wxString GuardZoneNames[3];

typedef enum RadarType {
    RT_BR24,
    RT_4G
} RadarType;

extern size_t convertMetersToRadarAllowedValue(int *range_meters, int units, RadarType radar_type);

#define DEFAULT_OVERLAY_TRANSPARENCY (5)
#define MIN_OVERLAY_TRANSPARENCY (0)
#define MAX_OVERLAY_TRANSPARENCY (10)

struct radar_control_settings {
    int      overlay_transparency;    // now 0-100, no longer a double
    bool     master_mode;
    bool     verbose;
    bool     auto_range_mode;
    int      range_index;
    int      display_option;
    int      display_mode;
    int      guard_zone;            // active zone (0 = none,1,2)
    int      guard_zone_threshold;  // How many blobs must be sent by radar before we fire alarm
    int      guard_zone_render_style;
    int      gain;
    int      interference_rejection;
    int      target_separation;
    int      noise_rejection;
    int      target_boost;
    int      filter_process;
    int      sea_clutter_gain;
    int      rain_clutter_gain;
    double   range_calibration;
    double   heading_correction;
    int      range_units;        // 0 = "Nautical miles"), 1 = "Statute miles", 2 = "Kilometers", 3 = "Meters"
    wxString radar_interface;        // IP address of interface to bind to (on UNIX)
    int      beam_width;
    int      max_age;
    int      draw_algorithm;
    int      scan_speed;
};

struct guard_zone_settings {
    int type;                   // 0 = circle, 1 = arc
    double inner_range;
    double outer_range;
    double start_bearing;
    double end_bearing;
};

struct scan_line {
    int range;                  // range of this scan line in decimeters
    wxDateTime age;             // how old this scan line is. We keep old scans on-screen for a while
    double heading;             // heading of boat at time of reception
    UINT8 data[512];          // radar return strength
};

//    Forward definitions
class RadarDataReceiveThread;
class RadarCommandReceiveThread;
class RadarReportReceiveThread;
class BR24ControlsDialog;
class GuardZoneDialog;
class GuardZoneBogey;
class BR24DisplayOptionsDialog;

//ofstream outfile("C:/ProgramData/opencpn/BR24DataDump.dat",ofstream::binary);

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#define BR24RADAR_TOOL_POSITION    -1          // Request default positioning of toolbar tool

class br24radar_pi : public opencpn_plugin_18
{
public:
    br24radar_pi(void *ppimgr);

//    The required PlugIn Methods
    int Init(void);
    bool DeInit(void);

    int GetAPIVersionMajor();
    int GetAPIVersionMinor();
    int GetPlugInVersionMajor();
    int GetPlugInVersionMinor();

    wxBitmap *GetPlugInBitmap();
    wxString GetCommonName();
    wxString GetShortDescription();
    wxString GetLongDescription();

//    The required override PlugIn Methods
    bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);
    bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
    void SetPositionFix(PlugIn_Position_Fix &pfix);
    void SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix);
    void SetCursorLatLon(double lat, double lon);
    void OnContextMenuItemCallback(int id);
    void SetNMEASentence(wxString &sentence);

    void SetDefaults(void);
    int GetToolbarToolCount(void);
    void OnToolbarToolCallback(int id);
    void ShowPreferencesDialog(wxWindow* parent);

// Other public methods

    void OnBR24ControlDialogClose();         // Control dialog
    void SetDisplayMode(int mode);
    void UpdateDisplayParameters(void);

    void SetBR24ControlsDialogX(long x) {
        m_BR24Controls_dialog_x = x;
    }
    void SetBR24ControlsDialogY(long y) {
        m_BR24Controls_dialog_y = y;
    }
    void SetBR24ControlsDialogSizeX(long sx) {
        m_BR24Controls_dialog_sx = sx;
    }
    void SetBR24ControlsDialogSizeY(long sy) {
        m_BR24Controls_dialog_sy = sy;
    }
    void Select_Guard_Zones(int zone);
    void OnGuardZoneDialogClose();
    void OnGuardZoneBogeyClose();
    void OnGuardZoneBogeyConfirm();

    void SetControlValue(ControlType controlType, int value);

    bool LoadConfig(void);
    bool SaveConfig(void);

    long GetRangeMeters();
    long GetOptimalRangeMeters();
    void SetRangeMeters(long range);

    radar_control_settings settings;

#define GUARD_ZONES (2)
    guard_zone_settings guardZones[GUARD_ZONES];

#define LINES_PER_ROTATION (4096) // 4G radars can generate up to 4096 lines per rotation
    scan_line                 m_scan_line[LINES_PER_ROTATION];

    BR24DisplayOptionsDialog *m_pOptionsDialog;
    BR24ControlsDialog       *m_pControlDialog;
    GuardZoneDialog          *m_pGuardZoneDialog;
    GuardZoneBogey           *m_pGuardZoneBogey;
    receive_statistics          m_statistics;

private:
    void TransmitCmd(UINT8 * msg, int size);
    void RadarTxOff(void);
    void RadarTxOn(void);
    void RadarStayAlive(void);
    void UpdateState(void);
    void DoTick(void);
    void Select_Clutter(int req_clutter_index);
    void Select_Rejection(int req_rejection_index);
    void RenderRadarOverlay(wxPoint radar_center, double v_scale_ppm, PlugIn_ViewPort *vp);
    void RenderSpectrum(wxPoint radar_center, double v_scale_ppm, PlugIn_ViewPort *vp);
    void RenderRadarBuffer(wxDC *pdc, int width, int height);
    void DrawRadarImage(int max_range, wxPoint radar_center);
    void RenderGuardZone(wxPoint radar_center, double v_scale_ppm, PlugIn_ViewPort *vp);
    void HandleBogeyCount(int *bogey_count);
    void draw_histogram_column(int x, int y);

    void CacheSetToolbarToolBitmaps(int bm_id_normal, int bm_id_rollover);
    void ShowRadarControl();

    wxFileConfig             *m_pconfig;
    wxWindow                 *m_parent_window;
    wxMenu                   *m_pmenu;

    int                       m_display_width, m_display_height;
    int                       m_tool_id;
    bool                      m_bShowIcon;
    wxBitmap                 *m_pdeficon;

    //    Controls added to Preferences panel
    wxCheckBox               *m_pShowIcon;

    RadarDataReceiveThread   *m_dataReceiveThread;
    RadarCommandReceiveThread *m_commandReceiveThread;
    RadarReportReceiveThread *m_reportReceiveThread;

    SOCKET                    m_radar_socket;

    long                      m_BR24Controls_dialog_sx, m_BR24Controls_dialog_sy ;
    long                      m_BR24Controls_dialog_x, m_BR24Controls_dialog_y ;

    long                      m_Guard_dialog_sx, m_Guard_dialog_sy ;
    long                      m_Guard_dialog_x, m_Guard_dialog_y ;


    wxBitmap                 *m_ptemp_icon;
//    wxLogWindow              *m_plogwin;
    int                       m_sent_bm_id_normal;
    int                       m_sent_bm_id_rollover;

    volatile bool             m_quit;

    int                       m_hdt_source;
    int                       m_hdt_prev_source;
    double                    m_var;

    NMEA0183                  m_NMEA0183;

    double                    llat, llon, ulat, ulon, dist_y, pix_y, v_scale_ppm;
};

class RadarDataReceiveThread: public wxThread
{

public:

    RadarDataReceiveThread(br24radar_pi *ppi, volatile bool * quit)
    : wxThread(wxTHREAD_JOINABLE)
    , pPlugIn(ppi)
    , m_quit(quit)
    {
//      wxLogMessage(_T("BR24 radar thread starting for multicast address %ls port %ls"), m_ip.c_str(), m_service_port.c_str());
      Create(1024 * 1024);
    };

    ~RadarDataReceiveThread(void);
    void *Entry(void);

    void OnExit(void);

private:
    void process_buffer(radar_frame_pkt * packet, int len);

    br24radar_pi      *pPlugIn;
    wxString           m_ip;
    volatile bool    * m_quit;
    wxIPV4address      m_myaddr;
};

class RadarCommandReceiveThread: public wxThread
{

public:

    RadarCommandReceiveThread(br24radar_pi *ppi, volatile bool * quit)
    : wxThread(wxTHREAD_JOINABLE)
    , pPlugIn(ppi)
    , m_quit(quit)
    {
      Create(64 * 1024);
    };

    ~RadarCommandReceiveThread(void);
    void *Entry(void);
    void OnExit(void);

private:
    br24radar_pi      *pPlugIn;
    wxString           m_ip;
    volatile bool    * m_quit;
    wxIPV4address      m_myaddr;
};

class RadarReportReceiveThread: public wxThread
{

public:

    RadarReportReceiveThread(br24radar_pi *ppi, volatile bool * quit)
    : wxThread(wxTHREAD_JOINABLE)
    , pPlugIn(ppi)
    , m_quit(quit)
    {
      Create(64 * 1024);
    };

    ~RadarReportReceiveThread(void);
    void *Entry(void);
    void OnExit(void);

private:
    void ProcessIncomingReport( UINT8 * command, int len );

    br24radar_pi      *pPlugIn;
    wxString           m_ip;
    volatile bool    * m_quit;
    wxIPV4address      m_myaddr;
};

//----------------------------------------------------------------------------------------------------------
//    BR24Radar DisplayOptions Dialog Specification
//----------------------------------------------------------------------------------------------------------

class BR24DisplayOptionsDialog: public wxDialog
{
    DECLARE_CLASS(BR24DisplayOptionsDialog)
    DECLARE_EVENT_TABLE()

public:

    BR24DisplayOptionsDialog();

    ~BR24DisplayOptionsDialog();
    void Init();

    bool Create(wxWindow *parent, br24radar_pi *ppi);

    void CreateDisplayOptions();

private:
    void OnClose(wxCloseEvent& event);
    void OnIdOKClick(wxCommandEvent& event);
    void OnRangeUnitsClick(wxCommandEvent& event);
    void OnDisplayOptionClick(wxCommandEvent& event);
    void OnIntervalSlider(wxCommandEvent& event);
    void OnDisplayModeClick(wxCommandEvent& event);
    void OnGuardZoneStyleClick(wxCommandEvent& event);
    void OnHeading_Calibration_Value(wxCommandEvent& event);

    wxWindow          *pParent;
    br24radar_pi      *pPlugIn;

    // DisplayOptions
    wxRadioBox        *pRangeUnits;
    wxRadioBox        *pOverlayDisplayOptions;
    wxRadioBox        *pDisplayMode;
    wxRadioBox        *pGuardZoneStyle;
    wxSlider          *pIntervalSlider;
    wxTextCtrl        *pText_Heading_Correction_Value;
};


//----------------------------------------------------------------------------------------------------------
//    BR24Radar Control Dialog Helpers Specification
//----------------------------------------------------------------------------------------------------------

class RadarControlButton: public wxButton
{
public:
    RadarControlButton()
    {

    };

    RadarControlButton(wxWindow *parent,
                       wxWindowID id,
                       const wxString& label,
                       br24radar_pi *ppi,
                       ControlType ct,
                       bool newHasAuto,
                       int newValue
                      )
    {
        Create(parent, id, label, wxDefaultPosition, g_buttonSize, 0, wxDefaultValidator, label);
        minValue = 0;
        maxValue = 100;
        value = 0;
        hasAuto = newHasAuto;
        pPlugIn = ppi;
        firstLine = label;
        names = 0;
        controlType = ct;
        if (hasAuto) {
            SetAuto();
        } else {
            SetValue(newValue);
        }

        this->SetFont(g_font);
    }

    // Set a new value, if it is in range. If not the value is ignored.
    // Computes a new label and a new technicalValue
    // The default conversion is technicalValue = (int) ((double) value * 255.0 / 100.0)
    virtual void SetValue(int value);
    virtual void SetAuto();

    const wxString  *names;

    wxString   firstLine;

    int        technicalValue; // value converted to what system needs
    br24radar_pi *pPlugIn;

    int        value;
    bool       isAuto;

    int        minValue;
    int        maxValue;
    bool       hasAuto;
    ControlType controlType;

};

class RadarRangeControlButton: public RadarControlButton
{
public:
    RadarRangeControlButton(wxWindow *parent,
                            wxWindowID id,
                            const wxString& label,
                            br24radar_pi *ppi
                           )
    {
        Create(parent, id, label, wxDefaultPosition, g_buttonSize, 0, wxDefaultValidator, label);
        minValue = 0;
        maxValue = 0;
        value = -1; // means: never set
        hasAuto = true;
        pPlugIn = ppi;
        firstLine = label;
        names = 0;
        controlType = CT_RANGE;

        this->SetFont(g_font);

        isAuto = ppi->settings.auto_range_mode;
    }

    int auto_range_index;

    virtual void SetValue(int value);
    virtual void SetAuto();

    int SetValueInt(int value);
};

//----------------------------------------------------------------------------------------------------------
//    BR24Radar Control Dialog Specification
//----------------------------------------------------------------------------------------------------------
class BR24ControlsDialog: public wxDialog
{
    DECLARE_CLASS(BR24ControlsDialog)
    DECLARE_EVENT_TABLE()

public:

    BR24ControlsDialog();

    ~BR24ControlsDialog();
    void Init();

    bool Create(wxWindow *parent, br24radar_pi *ppi, wxWindowID id = wxID_ANY,
                const wxString& caption = _("Radar"),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE & ~(wxMAXIMIZE_BOX)
                );

    void CreateControls();
    void SetRangeIndex(size_t index);
    void SetAutoRangeIndex(size_t index);
    void UpdateGuardZoneState();

    wxStaticText       *tStatistics;

private:
    void OnClose(wxCloseEvent& event);
    void OnIdOKClick(wxCommandEvent& event);
    void OnMove(wxMoveEvent& event);
    void OnSize(wxSizeEvent& event);

    void OnPlusTenClick(wxCommandEvent& event);
    void OnPlusClick(wxCommandEvent& event);
    void OnBackClick(wxCommandEvent& event);
    void OnMinusClick(wxCommandEvent& event);
    void OnMinusTenClick(wxCommandEvent& event);
    void OnAutoClick(wxCommandEvent& event);

    void OnAdvancedBackButtonClick(wxCommandEvent& event);
    void OnAdvancedButtonClick(wxCommandEvent& event);

    void OnRadarControlButtonClick(wxCommandEvent& event);

    void OnZone1ButtonClick(wxCommandEvent &event);
    void OnZone2ButtonClick(wxCommandEvent &event);

    void EnterEditMode(RadarControlButton * button);

    wxWindow          *pParent;
    br24radar_pi      *pPlugIn;

    wxBoxSizer        *topSizer;
    wxBoxSizer        *editBox;
    wxBoxSizer        *advancedBox;
    wxBoxSizer        *controlBox;

    wxBoxSizer        *fromBox; // If on edit control, this is where the button is from


    // Edit Controls

    RadarControlButton *fromControl; // Only set when in edit mode

    // The 'edit' control has these buttons:
    wxButton           *bBack;
    wxButton           *bPlusTen;
    wxButton           *bPlus;
    wxStaticText       *tValue;
    wxButton           *bMinus;
    wxButton           *bMinusTen;
    wxButton           *bAuto;

    // Advanced controls
    wxButton           *bAdvancedBack;
    RadarControlButton *bTransparency;
    RadarControlButton *bInterferenceRejection;
    RadarControlButton *bTargetSeparation;
    RadarControlButton *bNoiseRejection;
    RadarControlButton *bTargetBoost;
    RadarControlButton *bScanSpeed;
    RadarControlButton *bScanAge;

    // Show Controls

    RadarRangeControlButton *bRange;
    RadarControlButton *bGain;
    RadarControlButton *bSea;
    RadarControlButton *bRain;
    wxButton           *bAdvanced;
    wxButton           *bGuard1;
    wxButton           *bGuard2;
};

/*
 =======================================================================================================================
    BR24Radar Guard Zone Dialog Specification ;
 =======================================================================================================================
 */
class GuardZoneDialog :    public wxDialog
{
    DECLARE_CLASS(GuardZoneDialog)
    DECLARE_EVENT_TABLE()

public:
    GuardZoneDialog();

    ~GuardZoneDialog();
    void    Init();

    bool    Create
            (
                wxWindow        *parent,
                br24radar_pi    *ppi,
                wxWindowID      id = wxID_ANY,
                const wxString  &m_caption = _(" Guard Zone Control"),
                const wxPoint   &pos = wxDefaultPosition,
                const wxSize    &size = wxDefaultSize,
                long            style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU
            );


    void    CreateControls();
    void    OnContextMenuGuardCallback(double mark_rng, double mark_brg);
    void    OnGuardZoneDialogShow(int zone);

private:
    void            SetVisibility();
    void            OnGuardZoneModeClick(wxCommandEvent &event);
    void            OnInner_Range_Value(wxCommandEvent &event);
    void            OnOuter_Range_Value(wxCommandEvent &event);
    void            OnStart_Bearing_Value(wxCommandEvent &event);
    void            OnEnd_Bearing_Value(wxCommandEvent &event);
    void            OnClose(wxCloseEvent &event);
    void            OnIdOKClick(wxCommandEvent &event);

    wxWindow        *pParent;
    br24radar_pi    *pPlugIn;

    /* Controls */
    wxTextCtrl      *pZoneNumber;
    wxRadioBox      *pGuardZoneType;
    wxTextCtrl      *pInner_Range;
    wxTextCtrl      *pOuter_Range;
    wxTextCtrl      *pStart_Bearing_Value;
    wxTextCtrl      *pEnd_Bearing_Value;
};

/*
 =======================================================================================================================
    BR24Radar Guard Zone Bogey Dialog Specification ;
 =======================================================================================================================
 */
class GuardZoneBogey :    public wxDialog
{
    DECLARE_CLASS(GuardZoneBogey)
    DECLARE_EVENT_TABLE()

public:
    GuardZoneBogey();

    ~GuardZoneBogey();
    void    Init();

    bool    Create
            (
                wxWindow        *parent,
                br24radar_pi    *ppi,
                wxWindowID      id = wxID_ANY,
                const wxString  &m_caption = _("Guard Zone Active"),
                const wxPoint   &pos = wxDefaultPosition,
                const wxSize    &size = wxDefaultSize,
                long            style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU
            );


    void    CreateControls();
    void    SetBogeyCount(int *bogey_count, int next_alarm);

private:
    void            OnClose(wxCloseEvent &event);
    void            OnIdConfirmClick(wxCommandEvent &event);
    void            OnIdCloseClick(wxCommandEvent &event);

    wxWindow        *pParent;
    br24radar_pi    *pPlugIn;

    /* Controls */
    wxStaticText    *pBogeyCountText;
};

#endif
