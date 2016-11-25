
// Baker: All cvars in one place, cvar.h externs them.  cvar.c has the variables.
// Note: Textpad has ability to do sort starting at character #35 per line easily
//       so you can sort the list by alpha instead of by category.

// Remember, we cannot ifdef away any of these to ensure they preserve.

CVAR_DEF( Movie_Init  , AVI       ,  DEP_AVI  , capture_codec             , "capturevideo_codec"      , "auto"    , CVAR_ARCHIVE      , CaptureCodec_Validate,  ""			)
CVAR_DEF( Movie_Init  , AVI       ,  DEP_AVI  , capture_console           , "capturevideo_console"    , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( Movie_Init  , AVI       ,  DEP_AVI  , capture_fps               , "capturevideo_fps"        , "30.0"    , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( Movie_Init  , AVI       ,  DEP_AVI  , capture_hack              , "capturevideo_hack"       , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( Movie_Init  , AVI       ,  DEP_AVI  , capture_mp3               , "capturevideo_mp3"        , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( Movie_Init  , AVI       ,  DEP_AVI  , capture_mp3_kbps          , "capturevideo_mp3_kbps"   , "128"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , CLIENT    ,  DEP_NONE , cl_autodemo               , "cl_autodemo"             , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , CLIENT    ,  DEP_NONE , cl_backspeed              , "cl_backspeed"            , "400"     , CVAR_ARCHIVE      , NULL                 ,  ""            ) // Baker: Always run default, was 200
CVAR_DEF( CL_Init     , CLIENT    ,  DEP_NONE , cl_color                  , "_cl_color"               , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , CLIENT    ,  DEP_NONE , cl_forwardspeed           , "cl_forwardspeed"         , "400"     , CVAR_ARCHIVE      , NULL                 ,  ""            ) // Baker: Always run default, was 200
CVAR_DEF( CL_Init     , CLIENT    ,  DEP_NONE , cl_name                   , "_cl_name"                , "player"  , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , CLIENT    ,  DEP_NONE , cl_nolerp                 , "cl_nolerp"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , CLIENT    ,  DEP_NONE , cl_shownet                , "cl_shownet"              , "0"       , CVAR_NONE         , NULL                 ,  ""            ) // can be 0, 1, or 2
CVAR_DEF( CL_Init     , CLIENT    ,  DEP_NONE , cl_sidespeed              , "cl_sidespeed"            , "350"     , CVAR_ARCHIVE      , NULL                 ,  ""            )

CVAR_DEF( Utilities_Init, CLIENT,  DEP_NONE , install_depot_source, "install_depot_source"             , "http://www.quaddicted.com/filebase"       , CVAR_NONE      , NULL                 ,  ""            )


CVAR_DEF( COM_InitFilesystem,HOST,  DEP_NONE , cmdline                   , "cmdline"                 , ""        , CVAR_SERVERINFO   , NULL                 ,  ""            )
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , developer                 , "developer"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , devstats                  , "devstats"                , "0"       , CVAR_NONE         , NULL                 ,  ""            ) // johnfitz -- track developer statistics that vary every frame
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , neuralstats               , "neuralstats"             , "1"       , CVAR_NONE         , NULL                 ,  ""            ) // stephenkoren -- display debug info for neural network
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , neuralgraph               , "neuralgraph"             , "1"       , CVAR_NONE         , NULL                 ,  ""            ) // stephenkoren -- display debug graph for neural network
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , host_framerate            , "host_framerate"          , "0"       , CVAR_NONE         , NULL                 ,  ""            ) // set for slow motion
//CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , host_max_edicts           , "max_edicts"              , "0"       , CVAR_NONE         , Max_Edicts_f         ,  ""            ) // Baker: 0 = automatic
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , host_maxfps               , "host_maxfps"             , "72"      , CVAR_ARCHIVE      , NULL                 ,  ""            ) // johnfitz
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , host_sleep                , "host_sleep"              , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            ) // johnfitz
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , host_speeds               , "host_speeds"             , "0"       , CVAR_NONE         , NULL                 ,  ""            ) // set for running times
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , host_startdemos           , "host_startdemos"         , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            ) // Baker
CVAR_DEF( Host_Init   , HOST      ,  DEP_NONE , host_timescale            , "host_timescale"          , "0"       , CVAR_NONE         , NULL                 ,  ""            ) // johnfitz
CVAR_DEF( NET_Init    , HOST      ,  DEP_NONE , hostname                  , "hostname"                , "UNNAMED" , CVAR_NONE         , NULL                 ,  ""            ) // Server, really, a client-only doesn't need this
CVAR_DEF( COM_InitFilesystem,HOST      ,  DEP_NONE , registered                , "registered"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , cfg_unbindall             , "cfg_unbindall"           , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , cl_anglespeedkey          , "cl_anglespeedkey"        , "1.5"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , cl_maxpitch               , "cl_maxpitch"             , "90"      , CVAR_CLIENT | CVAR_ARCHIVE , NULL, ""           ) //johnfitz -- variable pitch clamping
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , cl_minpitch               , "cl_minpitch"             , "-90"     , CVAR_CLIENT | CVAR_ARCHIVE , NULL, ""           ) //johnfitz -- variable pitch clamping
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , cl_movespeedkey           , "cl_movespeedkey"         , "2.0"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , cl_pitchspeed             , "cl_pitchspeed"           , "150"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , cl_upspeed                , "cl_upspeed"              , "200"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , cl_yawspeed               , "cl_yawspeed"             , "140"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , in_joystick               , "joystick"                , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_name                  , "joyname"                 , "joystick", CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_advanced              , "joyadvanced"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_advaxisx              , "joyadvaxisx"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_advaxisy              , "joyadvaxisy"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_advaxisz              , "joyadvaxisz"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_advaxisr              , "joyadvaxisr"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_advaxisu              , "joyadvaxisu"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_advaxisv              , "joyadvaxisv"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_forwardthreshold      , "joyforwardthreshold"     , "0.15"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_sidethreshold         , "joysidethreshold"        , "0.15"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_flysensitivity        , "joyflysensitivity"       , "-1.0"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_flythreshold          , "joyflythreshold"         , "0.15"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_pitchthreshold        , "joypitchthreshold"       , "0.15"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_yawthreshold          , "joyyawthreshold"         , "0.15"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_forwardsensitivity    , "joyforwardsensitivity"   , "-1.0"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_sidesensitivity       , "joysidesensitivity"      , "-1.0"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_pitchsensitivity      , "joypitchsensitivity"     , "1.0"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_yawsensitivity        , "joyyawsensitivity"       , "-1.0"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_wwhack1               , "joywwhack1"              , "0.0"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Input_Joystick_Init, INPUT     ,  DEP_NONE , joy_wwhack2               , "joywwhack2"              , "0.0"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , lookspring                , "lookspring"              , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUT     ,  DEP_NONE , lookstrafe                , "lookstrafe"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUTMOUSE,  DEP_NONE , in_freelook               , "in_freelook"             , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUTMOUSE,  DEP_NONE , in_nomouse                , "nomouse"                 , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUTMOUSE,  DEP_NONE , m_forward                 , "m_forward"               , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUTMOUSE,  DEP_NONE , m_filter                  , "m_filter"                , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUTMOUSE,  DEP_NONE , m_pitch                   , "m_pitch"                 , "0.022"   , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUTMOUSE,  DEP_NONE , m_side                    , "m_side"                  , "0.8"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUTMOUSE,  DEP_NONE , m_yaw                     , "m_yaw"                   , "0.022"   , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUTMOUSE,  DEP_NONE , sensitivity               , "sensitivity"             , "3"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , INPUTMOUSE,  DEP_NONE , in_keypad                 , "cl_keypad"               , "1"       , CVAR_NONE         , NULL                 ,  ""            )


CVAR_DEF( Mod_Init    , MODEL     ,  DEP_GL   , gl_external_lits          , "external_lits"           , "1"       , CVAR_NONE         , Host_Changelevel_Required_Msg,  ""    )
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_GL   , gl_external_textures      , "external_textures"       , "1"       , CVAR_NONE         , External_Textures_Change_f,  ""       )
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_GL   , gl_fbrighthack_list       , "r_fbrighthack_list"      , "progs/flame.mdl,progs/flame2.mdl,progs/boss.mdl" , CVAR_NONE , Mod_Flags_Refresh_f                 ,  ""            )
CVAR_DEF( TexMgr_Init , MODEL     ,  DEP_GL   , gl_greyscale              , "gl_greyscale"            , "0"       , CVAR_NONE         , TexMgr_GreyScale_f   ,  ""            )
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_GL   , gl_nocolormap_list        , "r_nocolormap_list"       , ""        , CVAR_NONE         , NULL                 ,  ""            ) // Under consideration: progs/eyes.mdl,progs/h_player.mdl,progs/gib1.mdl,progs/gib2.mdl,progs/gib3.mdl
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_GL   , gl_noshadow_list          , "r_noshadow_list"         , "progs/flame.mdl,progs/flame2.mdl,progs/bolt.mdl,progs/bolt1.mdl,progs/bolt2.mdl,progs/bolt3.mdl,progs/laser.mdl"     , CVAR_NONE     , Mod_Flags_Refresh_f ,  ""            )
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_GL   , gl_texprefix_envmap       , "r_texprefix_envmap"      , "envmap_" , CVAR_NONE         , Host_Changelevel_Required_Msg                 ,  ""            ) // sphere mapped
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_GL   , gl_texprefix_mirror       , "r_texprefix_mirror"      , "mirror"  , CVAR_NONE         , Host_Changelevel_Required_Msg                 ,  ""            ) // Mirror texture
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_GL   , gl_texprefix_scrollx      , "r_texprefix_scrollx"     , "scrollx" , CVAR_NONE         , Host_Changelevel_Required_Msg                 ,  ""            ) // Scroll left -> right
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_GL   , gl_texprefix_scrolly      , "r_texprefix_scrolly"     , "scrolly" , CVAR_NONE         , Host_Changelevel_Required_Msg                 ,  ""            ) // Scroll up -> down
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_MIRROR  , gl_mirroralpha            , "r_mirroralpha"           , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Host_Init   , MODEL     ,  DEP_NONE , model_external_vis        , "external_vis"            , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , MODEL     ,  DEP_NONE , r_lavaalpha               , "r_lavaalpha"             , "1"       , CVAR_ARCHIVE | CVAR_CLIENT , NULL, ""           )
CVAR_DEF( R_Init      , MODEL     ,  DEP_NONE , r_nolerp_list             , "r_nolerp_list"           , "progs/flame.mdl,progs/flame2.mdl,progs/braztall.mdl,progs/brazshrt.mdl,progs/longtrch.mdl,progs/flame_pyre.mdl,progs/v_saw.mdl,progs/v_xfist.mdl,progs/h2stuff/newfire.mdl"      , CVAR_NONE     , Mod_Flags_Refresh_f,  ""            )
CVAR_DEF( R_Init      , MODEL     ,  DEP_NONE , r_slimealpha              , "r_slimealpha"            , "0.7"     , CVAR_ARCHIVE | CVAR_CLIENT , NULL, ""           )
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_NONE , r_texprefix_tele          , "r_texprefix_tele"        , "*tele"   , CVAR_NONE         , Host_Changelevel_Required_Msg,  ""    ) // To prevent r_wateralpha etc from affecting teleporters (Lava, Slime are own contents type but a tele isn't}.
CVAR_DEF( R_Init      , MODEL     ,  DEP_NONE , r_wateralpha              , "r_wateralpha"            , "0.5"     , CVAR_ARCHIVE | CVAR_CLIENT , NULL, ""           )
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_NONE , model_external_ents       , "external_ents"           , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_SW   , sw_sky_load_skyboxes      , "sw_sky_load_skyboxes"    , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            ) // Baker
CVAR_DEF( Mod_Init    , MODEL     ,  DEP_NONE , cl_sky					  , "_cl_sky"				  , ""        , CVAR_ARCHIVE      , NULL                 ,  ""            ) // Baker
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_affinemodels           , "gl_affinemodels"         , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_clear                  , "gl_clear"                , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_cull                   , "gl_cull"                 , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_drawworld              , "r_drawworld"             , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_dynamic                , "r_dynamic"               , "1"       , CVAR_NONE         , NULL                 ,  ""            )

CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_farclip                , "gl_farclip"              , "16384"   , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Sky_Init    , SCENE     ,  DEP_GL   , gl_fastsky                , "r_fastsky"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_finish                 , "gl_finish"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_flashblend             , "gl_flashblend"           , "1"       , CVAR_ARCHIVE | CVAR_CLIENT , NULL        ,  ""           )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_flatlightstyles         , "r_flatlightstyles"       , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_fullbrights            , "gl_fullbrights"          , "1"       , CVAR_NONE         , GL_Fullbrights_f     ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_lightmap               , "r_lightmap"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_nocolors               , "gl_nocolors"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_norefresh              , "r_norefresh"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_oldskyleaf             , "r_oldskyleaf"            , "0"       , CVAR_NONE         , R_VisChanged         ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_oldwater               , "r_oldwater"              , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_overbright             , "gl_overbright"           , "1"       , CVAR_NONE         , GL_Overbright_f      ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_overbright_models      , "gl_overbright_models"    , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_particles              , "r_particles"             , "1"       , CVAR_ARCHIVE      , R_SetParticleTexture_f,  ""           ) // johnfitz
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_quadparticles          , "r_quadparticles"         , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            ) // johnfitz
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_shadows                , "r_shadows"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_showbboxes             , "r_showbboxes"            , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_showtris               , "r_showtris"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Sky_Init    , SCENE     ,  DEP_GL   , gl_sky_quality            , "r_sky_quality"           , "12"      , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Sky_Init    , SCENE     ,  DEP_GL   , gl_skyalpha               , "r_skyalpha"              , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Sky_Init    , SCENE     ,  DEP_GL   , gl_skyfog                 , "r_skyfog"                , "0.5"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_smoothmodels           , "gl_smoothmodels"         , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_stereo                 , "r_stereo"                , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_stereodepth            , "r_stereodepth"           , "128"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_subdivide_size         , "gl_subdivide_size"       , "128"     , CVAR_ARCHIVE      , Host_Changelevel_Required_Msg, ""     )
CVAR_DEF( TexMgr_Init , SCENE     ,  DEP_GL   , gl_texture_anisotropy     , "gl_texture_anisotropy"   , "1"       , CVAR_ARCHIVE      , TexMgr_Anisotropy_f  ,  ""            )
CVAR_DEF( SCR_Init    , SCENE     ,  DEP_GL   , gl_triplebuffer           , "gl_triplebuffer"         , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_waterquality           , "r_waterquality"          , "8"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , gl_waterripple            , "r_waterripple"           , "0"       , CVAR_ARCHIVE | CVAR_CLIENT, NULL         ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_GL   , gl_zfix                   , "gl_zfix"                 , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_clearcolor              , "r_clearcolor"            , "2"       , CVAR_ARCHIVE      , R_SetClearColor_f    ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_drawentities            , "r_drawentities"          , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_drawflat                , "r_drawflat"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_fullbright              , "r_fullbright"            , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_lerpmodels              , "r_lerpmodels"            , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_lerpmove                , "r_lerpmove"              , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_lockfrustum             , "r_lockfrustum"           , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_lockpvs                 , "r_lockpvs"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_novis                   , "r_novis"                 , "0"       , CVAR_NONE         , R_VisChanged         ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_speeds                  , "r_speeds"                , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_stains                  , "r_stains"                , "1"       , CVAR_ARCHIVE      , Stain_Change_f       ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_stains_fadeamount       , "r_stains_fadeamount"     , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_NONE , r_stains_fadetime         , "r_stains_fadetime"       , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( D_Init      , SCENE     ,  DEP_SW   , sw_d_mipscale             , "d_mipscale"              , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( D_Init      , SCENE     ,  DEP_SW   , sw_d_subdiv16             , "d_subdiv16"              , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_aliasstats           , "r_polymodelstats"        , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_aliastransadj        , "r_aliastransadj"         , "100"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_aliastransbase       , "r_aliastransbase"        , "200"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_ambient              , "r_ambient"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_draworder            , "r_draworder"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_dspeeds              , "r_dspeeds"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_graphheight          , "r_graphheight"           , "10"      , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_maxedges             , "r_maxedges"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_maxsurfs             , "r_maxsurfs"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_numedges             , "r_numedges"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_numsurfs             , "r_numsurfs"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_reportedgeout        , "r_reportedgeout"         , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_reportsurfout        , "r_reportsurfout"         , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( R_Init      , SCENE     ,  DEP_SW   , sw_r_timegraph            , "r_timegraph"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( D_Init      , SCENE     ,  DEP_SW   , sw_d_mipcap               , "d_mipcap"                , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SCR_Init    , SCREEN    ,  DEP_GL   , gl_conalpha               , "scr_conalpha"            , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            ) //johnfitz
CVAR_DEF( SCR_Init    , SCREEN    ,  DEP_GL   , gl_conscale               , "scr_conscale"            , "1"       , CVAR_ARCHIVE      , SCR_Conwidth_f       ,  ""            )
CVAR_DEF( SCR_Init    , SCREEN    ,  DEP_GL   , gl_conwidth               , "scr_conwidth"            , "0"       , CVAR_ARCHIVE      , SCR_Conwidth_f       ,  ""            )
CVAR_DEF( SCR_Init    , SCREEN    ,  DEP_GL   , gl_crosshairscale         , "scr_crosshairscale"      , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( SCR_Init    , SCREEN    ,  DEP_GL   , gl_menuscale              , "scr_menuscale"           , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( SCR_Init    , SCREEN    ,  DEP_GL   , gl_sbaralpha              , "scr_sbaralpha"           , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( SCR_Init    , SCREEN    ,  DEP_GL   , gl_sbarscale              , "scr_sbarscale"           , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_centertime            , "scr_centertime"          , "2"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SCR_Init    , SCREEN    ,  DEP_NONE , scr_clock                 , "scr_clock"               , "-1"      , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_con_notifylines       , "con_notifylines"         , "4"       , CVAR_NONE         , NULL                 ,  ""            )  //seconds
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_con_notifytime        , "con_notifytime"          , "3"       , CVAR_NONE         , NULL                 ,  ""            )  //seconds
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_conspeed              , "scr_conspeed"            , "300"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_crosshair             , "crosshair"               , "0"       , CVAR_ARCHIVE | CVAR_CLIENT , NULL        ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_logcenterprint        , "con_logcenterprint"      , "1"       , CVAR_NONE         , NULL                 ,  ""            ) //johnfitz
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_printspeed            , "scr_printspeed"          , "8"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_sbarcentered          , "scr_sbarcentered"        , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_scoreboard_pings      , "scoreboard_pings"        , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_showfps               , "scr_showfps"             , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_showpause             , "showpause"               , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_showram               , "showram"                 , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_showturtle            , "showturtle"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( CL_Init     , SCREEN    ,  DEP_NONE , scr_viewsize              , "viewsize"                , "100"     , CVAR_ARCHIVE | CVAR_CLIENT , NULL        ,  ""            )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , _snd_mixahead             , "_snd_mixahead"           , "0.1"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , ambient_fade              , "ambient_fade"            , "100"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , ambient_level             , "ambient_level"           , "0.3"     , CVAR_CLIENT       , NULL                 ,  ""            )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , bgmvolume                 , "bgmvolume"               , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( CDAudio_Init, SOUND     ,  DEP_NONE , external_music            , "external_music"          , "1"       , CVAR_ARCHIVE      , external_music_toggle_f,  ""          )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , nosound                   , "nosound"                 , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , precache                  , "precache"                , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , sfxvolume                 , "volume"                  , "0.7"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , snd_noextraupdate         , "snd_noextraupdate"       , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , snd_show                  , "snd_show"                , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( S_Init      , SOUND     ,  DEP_NONE , sndspeed                  , "sndspeed"                , "11025"   , CVAR_ARCHIVE      , S_Snd_Speed_Notify_f ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , serverprofile             , "serverprofile"           , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_autosave               , "sv_autosave"             , "1"       , CVAR_NONE		  , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_pausable               , "pausable"                , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sys_ticrate               , "sys_ticrate"             , "0.05"    , CVAR_NONE         , NULL                 ,  ""            ) // dedicated server
CVAR_DEF( SV_Init     , MODEL_PR  ,  DEP_NONE , sv_fix_func_train_angles  , "sv_fix_func_train_angles", "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_aim                    , "sv_aim"                  , "0.93"    , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_cheats                 , "sv_cheats"               , "1"       , CVAR_NONE | CVAR_SERVERINFO       , SV_Cheats_f ,  ""     )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_cullentities           , "sv_cullentities"         , "0"       , CVAR_NOTIFY | CVAR_SERVERINFO     , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_idealpitchscale        , "sv_idealpitchscale"      , "0.8"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_novis                  , "sv_novis"                , "0"       , CVAR_NOTIFY | CVAR_SERVERINFO     , NULL                 ,  ""            )

CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_rquake				  , "pq_rquake"					 , "0"       , CVAR_NONE						, NULL                 ,  "Single player entities appear with deathmatch scoreboard for RQuake"            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_chat_frags_to_talk	  , "pq_chat_frags_to_talk"		 , "0"       , CVAR_NONE						, NULL                 ,  "Minimum score for newly connected player to talk."            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_chat_color_change_delay , "pq_chat_color_change_mute"  , "0"       , CVAR_NONE						, NULL                 ,  "Sets a delay when a player changes color before they can talk.")
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_chat_connect_mute_seconds      ,	"pq_chat_time_to_talk"		 , "0"       , CVAR_NONE						, NULL                 ,  "Number of seconds before newly connecting player can talk, allows a mod ban file to kick in first."            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_chat_log_player_number , "pq_chat_log_player_number"  , "1"       , CVAR_NONE						, NULL                 ,  "For chat logging, put the player # in the log to avoid name changing scripts faking a name."            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_remove_cr		      , "pq_remove_cr"		     , "1"       , CVAR_NONE						, NULL                 ,  "Remove carriage returns from chat and console logging"            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_chat_to_log			  , "pq_chat_to_log"		     , "0"       , CVAR_NONE						, NULL                 ,  "Log player chat messages to the console log."            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_dedicated_dequake			  , "pq_dedicated_dequake"		     , "1"       , CVAR_NONE						, NULL                 ,  "Quake characters converted to ascii for legible dedicated console output."            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_filter_gibs     , "sv_filter_gibs"           , "0"      , CVAR_NONE     , Mod_Flags_Refresh_f,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_filter_gibs_list, "sv_filter_gibs_list"           , "progs/s_bubble.spr,progs/gib1.mdl,progs/gib2.mdl,progs/gib3.mdl,progs/h_player.mdl,progs/h_dog.mdl,progs/h_guard.mdl,progs/h_mega.mdl,progs/h_knight.mdl,progs/h_hellkn.mdl,progs/h_wizard.mdl,progs/h_ogre.mdl,progs/h_demon.mdl,progs/h_shal.mdl,progs/h_shams.mdl,progs/h_zombie.mdl"      , CVAR_NONE     , Mod_Flags_Refresh_f,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_full_invisibility   , "sv_full_invisibility"    , "0"       , CVAR_NONE | CVAR_SERVERINFO      , NULL                 ,  "Having invisibility ring conveys no information to other players."            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_map_rotation   , "sv_map_rotation"    , ""       , CVAR_NONE | CVAR_SERVERINFO      , SV_Map_Rotation_Refresh_f,  "Having invisibility ring conveys no information to other players."            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_privacy_ipmasking	  , "pq_privacy_ipmasking"		 , "0"       , CVAR_NONE						, NULL                 ,  "Sets ip privacy ... 0: Quake normal, 1: ip masked, 2: masked and test shows 'private', 3: private to all"            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_privacy_name			  , "pq_privacy_name"			 , "0"       , CVAR_NONE						, NULL                 ,  "Sets name privacy ... 0: Quake normal, 1: 'anonymous' externally, 2: anonymous in-game"            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_ping_rounding	      , "pq_ping_rounding"		     , "0"       , CVAR_NONE						, NULL                 ,  "Rounds ping displays to nearest 40 millisecond interval for external clients"            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_password               , "pq_password"                , ""       , CVAR_NONE						, NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_chat_word_filter       , "pq_chat_word_filter"        , "0"       , CVAR_NONE						, NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , rcon_password			  , "rcon_password"		         , ""       , CVAR_NONE						, NULL                 ,  "rcon_server password"            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , rcon_server				  , "rcon_server"		         , ""       , CVAR_NONE						, NULL                 ,  "Run commands connected as if at server console typing them."            )

CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_url_banfile			  , "sv_url_banfile"    , ""       , CVAR_NONE      , Admin_Banlist_URL_Changed_f,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_url_whitelist		  , "sv_url_whitelist"    , ""       , CVAR_NONE      , Admin_Whitelist_URL_Changed_f,  ""            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_max_sameip_connections , "sv_max_sameip_connections" , "4"       , CVAR_NONE, NULL,  "Maximum connections from same ip.  Keep in mind LAN games, so try to exclude local networks."            )
CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , sv_fileserver_port		  ,	"sv_fileserver_port" , "+10"       , CVAR_NONE, NULL,  "File server port."            )


#if 0


//No server uses these because it is annoying
//CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_chat_spam_grace		  , "pq_chat_spam_grace"		 , "0"       , CVAR_NONE						, NULL                 ,  "Talking twice within this number of seconds triggers throttling of a player's chat"            )
//CVAR_DEF( SV_Init     , SV        ,  DEP_NONE , pq_chat_spam_rate		  , "pq_chat_spam_rate"			 , "0"       , CVAR_NONE						, NULL                 ,  "Amount to slow chatting"            )








#endif

CVAR_DEF( NET_Init    , SVNET     ,  DEP_NONE , net_connecttimeout        , "net_connecttimeout"      , "10"      , CVAR_NONE         , NULL                 ,  ""            ) // From ProQuake - qkick/qflood protection
CVAR_DEF( NET_Init    , SVNET     ,  DEP_NONE , net_messagetimeout        , "net_messagetimeout"      , "300"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_accelerate             , "sv_accelerate"           , "10"      , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_altnoclip              , "sv_altnoclip"            , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            ) //johnfitz
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_bouncedownslopes       , "sv_bouncedownslopes"     , "0"       , CVAR_NOTIFY | CVAR_SERVERINFO     , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_friction               , "sv_friction"             , "4"       , CVAR_NOTIFY | CVAR_SERVERINFO     , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_gravity                , "sv_gravity"              , "800"     , CVAR_NOTIFY | CVAR_SERVERINFO     , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_maxspeed               , "sv_maxspeed"             , "320"     , CVAR_NOTIFY | CVAR_SERVERINFO     , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_maxvelocity            , "sv_maxvelocity"          , "2000"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_nostep                 , "sv_nostep"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_edgefriction           , "edgefriction"            , "2"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( SV_Init     , SVPHYSICS ,  DEP_NONE , sv_stopspeed              , "sv_stopspeed"            , "100"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_coop                   , "coop"                    , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_deathmatch             , "deathmatch"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_fraglimit              , "fraglimit"               , "0"       , CVAR_NOTIFY | CVAR_SERVERINFO , Host_Callback_Notify,  "" )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_gamecfg                , "gamecfg"                 , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_noexit                 , "noexit"                  , "0"       , CVAR_NOTIFY | CVAR_SERVERINFO , Host_Callback_Notify,  "" )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_nomonsters             , "nomonsters"              , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_samelevel              , "samelevel"               , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_saved1                 , "saved1"                  , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_saved2                 , "saved2"                  , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_saved3                 , "saved3"                  , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_saved4                 , "saved4"                  , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_savedgamecfg           , "savedgamecfg"            , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_scratch1               , "scratch1"                , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_scratch2               , "scratch2"                , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_scratch3               , "scratch3"                , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_scratch4               , "scratch4"                , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_skill                  , "skill"                   , "1"       , CVAR_NONE         , NULL                 ,  ""            )      // 0 - 3
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_teamplay               , "teamplay"                , "0"       , CVAR_NOTIFY | CVAR_SERVERINFO , Host_Callback_Notify,  "" )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_temp1                  , "temp1"                   , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVPR      ,  DEP_NONE , pr_timelimit              , "timelimit"               , "0"       , CVAR_NOTIFY | CVAR_SERVERINFO , Host_Callback_Notify,  "" )
CVAR_DEF( PR_Init     , SVVM      ,  DEP_NONE , vm_coop_enhancements      , "sv_coop_enhancements"    , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVVM      ,  DEP_NONE , vm_fishfix                , "sv_fix_fish_count_bug"   , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVVM      ,  DEP_NONE , vm_imp12hack              , "sv_fix_no_impulse12"     , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( PR_Init     , SVVM      ,  DEP_NONE , vm_imp12hack_exceptions   , "sv_fix_no_impulse12_exceptions"     , "ne_ruins"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_NONE , vid_gamma                 , "gamma"                   , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_GL   , vid_hardwaregamma         , "vid_hardwaregamma"       , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_GL   , vid_multisample           , "vid_multisample"         , "0"       , CVAR_ARCHIVE      , VID_Local_Multisample_f,  ""          )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_NONE , vid_contrast              , "contrast"                , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_NONE , vid_fullscreen            , "vid_fullscreen"          , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_NONE , vid_height                , "vid_height"              , "480"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_FREQ , vid_refreshrate           , "vid_refreshrate"         , "60"      , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_FREQ , vid_vsync                 , "vid_vsync"              , "0"       , CVAR_ARCHIVE      , VID_Local_Vsync_f   ,  ""            )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_NONE , vid_width                 , "vid_width"               , "640"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( VID_Init    , VIDEO     ,  DEP_NONE , vid_sound_thread		  , "vid_sound_thread"        , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( Chase_Init  , VIEW      ,  DEP_NONE , chase_active              , "chase_active"            , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Chase_Init  , VIEW      ,  DEP_NONE , chase_back                , "chase_back"              , "100"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Chase_Init  , VIEW      ,  DEP_NONE , chase_right               , "chase_right"             , "0"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( Chase_Init  , VIEW      ,  DEP_NONE , chase_up                  , "chase_up"                , "16"      , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_bob                    , "cl_bob"                  , "0.02"    , CVAR_ARCHIVE | CVAR_CLIENT , NULL        ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_bobcycle               , "cl_bobcycle"             , "0.6"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_bobside                , "cl_bobside"              , "0.02"    , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_bobsidecycle           , "cl_bobsidecycle"         , "0.9"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_bobsideup              , "cl_bobsideup"            , "0.5"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_bobup                  , "cl_bobup"                , "0.5"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_rollangle              , "cl_rollangle"            , "2.0"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_rollspeed              , "cl_rollspeed"            , "200"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_sidebobbing            , "cl_sidebobbing"          , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , cl_titledemos_list        , "cl_titledemos_list"      , ""        , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , r_drawviewmodel           , "r_drawviewmodel"         , "1"       , CVAR_CLIENT       , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , r_ortho					  , "r_ortho"				  , "0"       , CVAR_CLIENT       , NULL                 ,  ""            ) // stephenkoren
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , r_lavacshift              , "r_lavacolor"             , "255 80 0 150", CVAR_NONE    , View_LavaCshift_f    ,  ""             ) // Was 255 80 50 150
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , r_slimecshift             , "r_slimecolor"            , "0 25 5 150", CVAR_NONE       , View_SlimeCshift_f   ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , r_viewmodel_always        , "r_viewmodel_always"      , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , r_viewmodel_fov           , "r_viewmodel_fov"         , "90"      , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , r_viewmodel_winquake      , "r_viewmodel_winquake"    , "0"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , r_watercshift             , "r_watercolor"            , "130 80 50 128", CVAR_NONE    , View_WaterCshift_f   ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , r_waterwarp               , "r_waterwarp"             , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , scr_fov                   , "fov"                     , "90"      , CVAR_ARCHIVE | CVAR_CLIENT , NULL        ,  ""            ) // 10 - 170
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_centermove              , "v_centermove"            , "0.15"    , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_centerspeed             , "v_centerspeed"           , "500"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_cshiftpercent           , "gl_cshiftpercent"        , "100"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_gunkick                 , "v_gunkick"               , "1"       , CVAR_ARCHIVE      , NULL                 ,  ""            ) // johnfitz
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_idlescale               , "v_idlescale"             , "0"       , CVAR_CLIENT       , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_ipitch_cycle            , "v_ipitch_cycle"          , "1"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_ipitch_level            , "v_ipitch_level"          , "0.3"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_iroll_cycle             , "v_iroll_cycle"           , "0.5"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_iroll_level             , "v_iroll_level"           , "0.1"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_iyaw_cycle              , "v_iyaw_cycle"            , "2"       , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_iyaw_level              , "v_iyaw_level"            , "0.3"     , CVAR_NONE         , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_kickpitch               , "v_kickpitch"             , "0.6"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_kickroll                , "v_kickroll"              , "0.6"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_kicktime                , "v_kicktime"              , "0.5"     , CVAR_ARCHIVE      , NULL                 ,  ""            )
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_polyblend               , "gl_polyblend"            , "1"       , CVAR_ARCHIVE | CVAR_CLIENT , R_PolyBlendChanged_f, ""           )  // Baker: --> // JPG 3.30 - winquake version of r_polyblend
CVAR_DEF( View_Init   , VIEW      ,  DEP_NONE , v_polyblend_lite          , "v_polyblend_lite"        , "0"       , CVAR_ARCHIVE | CVAR_CLIENT , R_PolyBlendChanged_f, ""           )  // Baker: --> // JPG 3.30 - winquake version of r_polyblend


//stephenkoren -- added neural network cvars
CVAR_DEF( NQ_Init , CLIENT    , DEP_NONE  , nq_color_1                , "nq_color_1"              , "251"     , CVAR_NONE         , NULL                 , ""             ) // First color used within neural network debug drawing.
CVAR_DEF( NQ_Init , CLIENT    , DEP_NONE  , nq_color_2                , "nq_color_2"              , "192"     , CVAR_NONE         , NULL                 , ""             ) // Second color used within neural network debug drawing.
CVAR_DEF( NQ_Init , CLIENT    , DEP_NONE  , nq_alpha                  , "nq_alpha"                , "1"       , CVAR_NONE         , NULL                 , ""             ) // Alpha used within neural network debug drawing.
//stephenkoren
#undef CVAR_DEF
