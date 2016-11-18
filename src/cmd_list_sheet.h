

CMD_DEF (Chase_Init  , DEP_CLIENT  , "chase_mode"                , Chase_Mode_f				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+attack"                   , IN_AttackDown			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+back"                     , IN_BackDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+forward"                  , IN_ForwardDown			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+jump"                     , IN_JumpDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+klook"                    , IN_KLookDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+left"                     , IN_LeftDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+lookdown"                 , IN_LookdownDown			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+lookup"                   , IN_LookupDown			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+mlook"                    , IN_MLookDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+movedown"                 , IN_DownDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+moveleft"                 , IN_MoveleftDown			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+moveright"                , IN_MoverightDown			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+moveup"                   , IN_UpDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+right"                    , IN_RightDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+speed"                    , IN_SpeedDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+strafe"                   , IN_StrafeDown			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "+use"                      , IN_UseDown				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-attack"                   , IN_AttackUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-back"                     , IN_BackUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-forward"                  , IN_ForwardUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-jump"                     , IN_JumpUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-klook"                    , IN_KLookUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-left"                     , IN_LeftUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-lookdown"                 , IN_LookdownUp			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-lookup"                   , IN_LookupUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-mlook"                    , IN_MLookUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-movedown"                 , IN_DownUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-moveleft"                 , IN_MoveleftUp			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-moveright"                , IN_MoverightUp			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-moveup"                   , IN_UpUp					, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-right"                    , IN_RightUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-speed"                    , IN_SpeedUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-strafe"                   , IN_StrafeUp				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "-use"                      , IN_UseUp					, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "bestweapon"                , IN_BestWeapon			, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "impulse"                   , IN_Impulse				, ""			)
CMD_DEF (CL_InitInput, DEP_CLIENT  , "pq_fullpitch"              , IN_PQ_Full_Pitch_f		, ""			)

CMD_DEF (Input_Init  , DEP_CLIENT  , "force_centerview"          , Input_Force_CenterView_f	, ""			)
CMD_DEF (Input_Init  , DEP_CLIENT  , "in_info"                   , Input_Info_f				, ""			)

#ifdef _WIN32
CMD_DEF ((voidfunc_t)Input_Local_Joystick_Startup, DEP_CLIENT  , "joyadvancedupdate"         , Input_Local_Joy_AdvancedUpdate_f, ""		)
#endif // _WIN32

#ifdef GLQUAKE_RENDERER_SUPPORT
CMD_DEF (R_Init_Local, DEP_GL      , "envmap"				     , R_Envmap_f				, ""			)
CMD_DEF (Fog_Init    , DEP_GL      , "fog"                       , Fog_FogCommand_f			, ""			)
CMD_DEF (Fog_Init    , DEP_GL      , "fogex"                     , Fog_FogExCommand_f		, ""			)
#endif // GLQUAKE_RENDERER_SUPPORT

#ifdef WINQUAKE_RENDERER_SUPPORT
CMD_DEF (R_Init_Local, DEP_SW      , "fog"                       , Cmd_No_Command			, ""			) // Baker: CZG's honey spams this so we need it nulled out.
CMD_DEF (R_Init_Local, DEP_SW      , "fogex"                     , Cmd_No_Command		, ""			)
#endif // WINQUAKE_RENDERER_SUPPORT

CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "playmod"                   , FMOD_Play_f				, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "stopmod"                   , FMOD_Stop_f				, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "play2"                     , S_Play2_f				, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "playvol"                   , S_PlayVol				, ""			)

CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "gl_fogenable"				 , Cmd_No_Command			, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "gl_fogdensity"    		 , Cmd_No_Command			, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "gl_fogred"				 , Cmd_No_Command			, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "gl_foggreen"				 , Cmd_No_Command			, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "gl_fogblue"				 , Cmd_No_Command			, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "nextchase"				 , Cmd_No_Command			, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "gl_notrans"				 , Cmd_No_Command			, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "r_oldsky"					 , Cmd_No_Command			, ""			)
CMD_DEF (Nehahra_Init, DEP_CL_NEH  , "r_nospr32"				 , Cmd_No_Command			, ""			)

CMD_DEF (Sbar_Init   , DEP_CLIENT  , "+showscores"               , Sbar_ShowScores			, ""			)
CMD_DEF (Sbar_Init   , DEP_CLIENT  , "-showscores"               , Sbar_DontShowScores		, ""			)

CMD_DEF (PR_Init     , DEP_HOST__  , "edict"                     , ED_PrintEdict_f			, ""			)
CMD_DEF (PR_Init     , DEP_HOST__  , "edictcount"                , ED_Count					, ""			)
CMD_DEF (PR_Init     , DEP_HOST__  , "edicts"                    , ED_PrintEdicts			, ""			)
CMD_DEF (PR_Init     , DEP_HOST__  , "profile"                   , PR_Profile_f				, ""			)
CMD_DEF (PR_Init     , DEP_HOST__  , "qcexec"                    , PR_QC_Exec				, ""			)
CMD_DEF (PR_Init     , DEP_HOST__  , "qcfuncs"                   , PR_Listfunctions_f		, ""			)

CMD_DEF (Host_Init   , DEP_HOST__  , "begin"                     , Host_Begin_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "changelevel"               , Host_Changelevel_f		, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "color"                     , Host_Color_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "connect"                   , Host_Connect_f			, ""			)
//CMD_DEF (Host_Init   , DEP_HOST__  , "dir"                       , Dir_f						, ""			)
//CMD_DEF (Host_Init   , DEP_HOST__  , "download"                  , Downloads_Download_f		, ""			)
//CMD_DEF (Host_Init   , DEP_HOST__  , "downloadzip"               , Download_Remote_Zip_Install_f	, ""		)
CMD_DEF (Host_Init   , DEP_HOST__  , "fly"                       , Host_Fly_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "game"                      , Host_Game_f				, ""			) //johnfitz
CMD_DEF (Host_Init   , DEP_HOST__  , "give"                      , Host_Give_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "god"                       , Host_God_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "kill"                      , Host_Kill_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "load"                      , Host_Loadgame_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "map"                       , Host_Map_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "mapname"                   , Host_Mapname_f			, ""			) //johnfitz
CMD_DEF (Host_Init   , DEP_HOST__  , "name"                      , Host_Name_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "noclip"                    , Host_Noclip_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "notarget"                  , Host_Notarget_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "pause"                     , Host_Pause_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "ping"                      , Host_Ping_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "prespawn"                  , Host_PreSpawn_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "quit"                      , Host_Quit_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "reconnect"                 , Host_Reconnect_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "restart"                   , Host_Restart_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "save"                      , Host_Savegame_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "say"                       , Host_Say_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "say_team"                  , Host_Say_Team_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "spawn"                     , Host_Spawn_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "status"                    , Host_Status_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "tell"                      , Host_Tell_f				, ""			)
CMD_DEF (Host_Init   , DEP_HOST__  , "version"                   , Host_Version_f			, ""			)

CMD_DEF (Host_Init   , DEP_CLIENT  , "startdemos"                , Host_Startdemos_f		, ""			)
CMD_DEF (Host_Init   , DEP_CLIENT  , "stopdemo"                  , Host_Stopdemo_f			, ""			)
CMD_DEF (Host_Init   , DEP_NONE    , "_host_post_initialized"    , Host_Post_Initialization_f,""            )
CMD_DEF (Host_Init   , DEP_HOST    , "mcache"                    , Mod_Print				, ""			)
CMD_DEF (Host_Init   , DEP_HOST    , "models"                    , Mod_PrintEx				, ""			) // Baker -- another way to get to this

CMD_DEF (Host_Init   , DEP_HOST    , "freezeall"                 , Host_Freezeall_f			, ""			)
CMD_DEF (Host_Init   , DEP_HOST    , "sv_freezenonclients"       , Host_Legacy_FreezeAll_f	, ""			) // Baker -- another way to get to this

CMD_DEF (CL_Init     , DEP_CLIENT  , "cl_hints"                  , CL_Hints_List_f			, ""			)
CMD_DEF (CL_Init     , DEP_CLIENT  , "disconnect"                , CL_Disconnect_f			, ""			)
CMD_DEF (CL_Init     , DEP_CLIENT  , "entities"                  , CL_PrintEntities_f		, ""			)
CMD_DEF (CL_Init     , DEP_CLIENT  , "nextstartdemo"             , CL_PlayDemo_NextStartDemo_f , ""			)
CMD_DEF (CL_Init     , DEP_CLIENT  , "playdemo"                  , CL_PlayDemo_f			, ""			)
CMD_DEF (CL_Init     , DEP_CLIENT  , "record"                    , CL_Record_f				, ""			)
CMD_DEF (CL_Init     , DEP_CLIENT  , "stop"                      , CL_Stop_f				, ""			)
CMD_DEF (CL_Init     , DEP_CLIENT  , "timedemo"                  , CL_TimeDemo_f			, ""			)
CMD_DEF (CL_Init     , DEP_CLIENT  , "tracepos"                  , CL_Tracepos_f			, ""			) //johnfitz
CMD_DEF (CL_Init     , DEP_CLIENT  , "viewpos"                   , CL_Viewpos_f				, ""			) //johnfitz
CMD_DEF (CL_Init     , DEP_CLIENT  , "warppos"                   , CL_Warppos_f				, ""			)

CMD_DEF (M_Init      , DEP_CLIENT  , "menu_keys"                 , M_Menu_Keys_f			, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "menu_load"                 , M_Menu_Load_f			, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "menu_main"                 , M_Menu_Main_f			, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "menu_multiplayer"          , M_Menu_MultiPlayer_f		, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "menu_options"              , M_Menu_Options_f			, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "menu_quit"                 , M_Menu_Quit_f			, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "menu_save"                 , M_Menu_Save_f			, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "menu_setup"                , M_Menu_Setup_f			, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "menu_singleplayer"         , M_Menu_SinglePlayer_f	, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "menu_video"                , M_Menu_Video_f			, ""			)
CMD_DEF (M_Init      , DEP_CLIENT  , "togglemenu"                , M_ToggleMenu_f			, ""			)
CMD_DEF (M_Init      , DEP_NONE    , "help"                      , M_Menu_Help_f			, ""			)

CMD_DEF (SV_Init     , DEP_HOST__  , "sv_hints"                  , SV_Hints_List_f			, ""			) //johnfitz
CMD_DEF (SV_Init     , DEP_HOST__  , "sv_protocol"               , SV_Protocol_f			, ""			) //johnfitz

CMD_DEF (Key_Init    , DEP_CLIENT  , "bind"                      , Key_Bind_f				, ""			)
CMD_DEF (Key_Init    , DEP_CLIENT  , "bindlist"                  , Key_Bindlist_f			, ""			) //johnfitz
CMD_DEF (Key_Init    , DEP_CLIENT  , "binds"                     , Key_Bindlist_f			, ""			) // Baker --- alternate way to get here
CMD_DEF (Key_Init    , DEP_CLIENT  , "unbind"                    , Key_Unbind_f				, ""			)
CMD_DEF (Key_Init    , DEP_CLIENT  , "unbindall"                 , Key_Unbindall_f			, ""			)

CMD_DEF (Cmd_Init    , DEP_HOST__  , "alias"                     , Cmd_Alias_f				, ""			)
CMD_DEF (Cmd_Init    , DEP_HOST__  , "cmd"                       , Cmd_ForwardToServer		, ""			)
CMD_DEF (Cmd_Init    , DEP_HOST__  , "cmdlist"                   , Cmd_List_f				, ""			) //johnfitz
CMD_DEF (Cmd_Init    , DEP_HOST__  , "echo"                      , Cmd_Echo_f				, ""			)
CMD_DEF (Cmd_Init    , DEP_HOST__  , "exec"                      , Cmd_Exec_f				, ""			)
CMD_DEF (Cmd_Init    , DEP_HOST__  , "files"                     , FS_List_Open_f			, ""			)
CMD_DEF (Cmd_Init    , DEP_HOST__  , "stuffcmds"                 , Cmd_StuffCmds_f			, ""			)
CMD_DEF (Cmd_Init    , DEP_HOST__  , "unalias"                   , Cmd_Unalias_f			, ""			) //johnfitz
CMD_DEF (Cmd_Init    , DEP_HOST__  , "unaliasall"                , Cmd_Unaliasall_f			, ""			) //johnfitz
CMD_DEF (Cmd_Init    , DEP_HOST__  , "wait"                      , Cmd_Wait_f				, ""			)

//CMD_DEF (NET_Init    , DEP_HOST__  , "ban"                       , NET_Ban_f				, ""			)
CMD_DEF (NET_Init    , DEP_HOST__  , "net_stats"                 , NET_Stats_f				, ""			)

// new
CMD_DEF (NET_Init    , DEP_HOST__  , "ban"						 , NET_Ban_f				, "Ban by player #")
CMD_DEF (NET_Init    , DEP_HOST__  , "banip"                     , Admin_Ban_Ip_f				, "Ban by ip"   )
CMD_DEF (NET_Init    , DEP_HOST__  , "banlist"                   , Admin_Ban_List_f		    , "List adverse actions" )
CMD_DEF (NET_Init    , DEP_HOST__  , "whitelist"                 , Admin_White_List_f		    , "List adverse actions" )

CMD_DEF (NET_Init    , DEP_HOST__  , "kick"                      , Host_Kick_f				, "Kick a player")
CMD_DEF (NET_Init    , DEP_HOST__  , "mute"                      , Admin_Mute_f				, "Mute by player #")
CMD_DEF (NET_Init    , DEP_HOST__  , "lockserver"			     , Admin_Lock_f				, "Prevent new connections")
CMD_DEF (NET_Init    , DEP_HOST__  , "unlockserver"			     , Admin_UnLock_f				, "Re-enable new connections")

CMD_DEF (NET_Init    , DEP_HOST__  , "test"                      , Test_f					, ""			)
CMD_DEF (NET_Init    , DEP_HOST__  , "test2"                     , Test2_f					, ""			)
CMD_DEF (NET_Init    , DEP_HOST__  , "slist"                     , NET_Slist_f			    , ""			)
CMD_DEF (NET_Init    , DEP_HOST__  , "listen"                    , NET_Listen_f				, ""			)
CMD_DEF (NET_Init    , DEP_HOST__  , "maxplayers"                , MaxPlayers_f				, ""			)
CMD_DEF (NET_Init    , DEP_HOST__  , "port"						 , NET_Port_f				, ""			)

CMD_DEF (NET_Init    , DEP_HOST__  , "rcon"						 , Rcon_f				    , "Remote console"			)

CMD_DEF (Cvar_Init   , DEP_NONE    , "cvarlist"                  , Cvar_List_f				, ""			)
CMD_DEF (Cvar_Init   , DEP_NONE    , "cycle"                     , Cvar_Cycle_f				, ""			)
CMD_DEF (Cvar_Init   , DEP_NONE    , "dec"                       , Cvar_Dec_f				, ""			)
CMD_DEF (Cvar_Init   , DEP_CLIENT  , "resetall"                  , Cvar_ResetAll_f			, ""			)
CMD_DEF (Cvar_Init   , DEP_NONE    , "resetcfg"                  , Cvar_ResetCfg_f			, ""			)
CMD_DEF (Cvar_Init   , DEP_NONE    , "resetcvar"                 , Cvar_Reset_f				, ""			)
CMD_DEF (Cvar_Init   , DEP_CLIENT  , "toggle"                    , Cvar_Toggle_f			, ""			)
CMD_DEF (Cvar_Init   , DEP_NONE    , "inc"                       , Cvar_Inc_f				, ""			)

#ifdef SUPPORTS_AVI_CAPTURE
CMD_DEF (Movie_Init  , DEP_CLIENT  , "capturedemo"               , Movie_CaptureDemo_f		, ""			)
CMD_DEF (Movie_Init  , DEP_CLIENT  , "capturedemostop"           , Movie_Stop_Capturing		, ""			)
CMD_DEF (Movie_Init  , DEP_CLIENT  , "capturevideo"              , Movie_Capture_Toggle_f	, ""			)
#endif // SUPPORTS_AVI_CAPTURE

CMD_DEF (S_Init      , DEP_CL_NEH  , "playvol"                   , S_PlayVol				, ""			)
CMD_DEF (S_Init      , DEP_CLIENT  , "play"                      , S_Play_f					, ""			)
CMD_DEF (S_Init      , DEP_CLIENT  , "soundinfo"                 , S_SoundInfo_f			, ""			)
CMD_DEF (S_Init      , DEP_CLIENT  , "soundlist"                 , S_SoundList				, ""			)
CMD_DEF (S_Init      , DEP_CLIENT  , "stopsound"                 , S_StopAllSoundsC			, ""			)

#ifdef GLQUAKE_RENDERER_SUPPORT
CMD_DEF (TexMgr_Init , DEP_GL      , "gl_describetexturemodes"   , TexMgr_DescribeTextureModes_f	, ""		)
CMD_DEF (TexMgr_Init , DEP_GL      , "gl_texturemode"            , TexMgr_TextureMode_f		, ""			)
CMD_DEF (TexMgr_Init , DEP_GL      , "imagedump"                 , TexMgr_Imagedump_f		, ""			)
CMD_DEF (TexMgr_Init , DEP_GL      , "imagelist"                 , TexMgr_Imagelist_f		, ""			)
CMD_DEF (TexMgr_Init , DEP_MODEL   , "texreload"                 , TexMgr_ReloadImages_f	, ""			)
CMD_DEF (TexMgr_Init , DEP_GL      , "textures"                  , TexMgr_Imagelist_f		, ""			) // Baker another way to get to this
#endif // GLQUAKE_RENDERER_SUPPORT

CMD_DEF (R_Init     , DEP_CLIENT  , "timerefresh"               , R_TimeRefresh_f			, ""			)
CMD_DEF (R_Init     , DEP_CLIENT  , "pointfile"                 , R_ReadPointFile_f			, ""			)


CMD_DEF (Con_Init   , DEP_CLIENT  , "clear"                     , Con_Clear_f				, ""			)
CMD_DEF (Con_Init   , DEP_CLIENT  , "condump"                   , Con_Dump_f				, ""			) //johnfitz
CMD_DEF (Con_Init   , DEP_CLIENT  , "copy"                      , Con_Copy_f				, ""			)
CMD_DEF (Con_Init   , DEP_CLIENT  , "messagemode"               , Con_MessageMode_f			, ""			)
CMD_DEF (Con_Init   , DEP_CLIENT  , "messagemode2"              , Con_MessageMode2_f		, ""			)
CMD_DEF (Con_Init   , DEP_CLIENT  , "toggleconsole"             , Con_ToggleConsole_f		, ""			)


CMD_DEF (SCR_Init   , DEP_CLIENT  , "screenshot"                , SCR_ScreenShot_f			, ""			)
CMD_DEF (SCR_Init   , DEP_CLIENT  , "sizedown"                  , SCR_SizeDown_f			, ""			)
CMD_DEF (SCR_Init   , DEP_CLIENT  , "sizeup"                    , SCR_SizeUp_f				, ""			)


CMD_DEF (Lists_Init , DEP_CLIENT  , "configs"                   , List_Configs_f			, ""			) // Baker --- another way to get to this
CMD_DEF (Lists_Init , DEP_NONE    , "demos"                     , List_Demos_f				, ""			)
CMD_DEF (Lists_Init , DEP_NONE    , "games"                     , List_Games_f				, ""			) // as an alias to "mods" -- S.A. / QuakeSpasm
CMD_DEF (Lists_Init , DEP_CLIENT  , "keys"                      , List_Keys_f				, ""			)
CMD_DEF (Lists_Init , DEP_CLIENT  , "maps"                      , List_Maps_f				, ""			) //johnfitz
CMD_DEF (Lists_Init , DEP_CLIENT  , "mods"                      , List_Games_f				, ""			) //johnfitz
CMD_DEF (Lists_Init , DEP_CLIENT  , "mp3s"                      , List_MP3s_f				, ""			) //johnfitz
CMD_DEF (Lists_Init , DEP_NONE    , "saves"                     , List_Savegames_f			, ""			)
CMD_DEF (Lists_Init , DEP_CLIENT  , "skys"                      , List_Skys_f				, ""			) //johnfitz
CMD_DEF (Lists_Init , DEP_CLIENT  , "sounds"                    , List_Sounds_f				, ""			) // Baker --- another way to get to this

CMD_DEF (Recent_File_Init, DEP_CLIENT  , "folder"               , Recent_File_Show_f		, ""			)
CMD_DEF (Recent_File_Init, DEP_CLIENT  , "showfile"             , Recent_File_Show_f		, ""			)

#ifdef GLQUAKE_RENDERER_SUPPORT
CMD_DEF (VID_Init   , DEP_CLIENT  , "gl_info"                   , GL_Info_f					, ""			)
#endif // GLQUAKE_RENDERER_SUPPORT
CMD_DEF (VID_Init   , DEP_CLIENT  , "vid_restart"               , VID_Restart_f				, ""			)
CMD_DEF (VID_Init   , DEP_CLIENT  , "vid_test"                  , VID_Test					, ""			)

CMD_DEF (View_Init  , DEP_CLIENT  , "bf"                        , View_BonusFlash_f			, ""			)
CMD_DEF (View_Init  , DEP_CLIENT  , "centerview"                , View_StartPitchDrift		, ""			)
CMD_DEF (View_Init  , DEP_CLIENT  , "v_cshift"                  , View_cshift_f				, ""			)

// stephenkoren -- added neural commands
CMD_DEF (NQ_Init	, DEP_CLIENT  , "nq_start"					, NQ_Start					, ""			)
CMD_DEF (NQ_Init	, DEP_CLIENT  , "nq_end"					, NQ_End					, ""			)
CMD_DEF (NQ_Init	, DEP_CLIENT  , "nq_save"					, NQ_Save					, ""			)
CMD_DEF (NQ_Init	, DEP_CLIENT  , "nq_load"					, NQ_Load					, ""			)
CMD_DEF (NQ_Init	, DEP_CLIENT  , "nq_forcetimeout"			, NQ_ForceTimeout			, ""			)

#ifdef GLQUAKE_RENDERER_SUPPORT
CMD_DEF (Entity_Inspector_Init, DEP_CLIENT  , "tool_inspector"   , Tool_Inspector_f			, ""			)
CMD_DEF (TexturePointer_Init, DEP_CLIENT  , "tool_texturepointer", Texture_Pointer_f		, ""			)
#endif // GLQUAKE_RENDERER_SUPPORT

CMD_DEF (COM_InitFilesystem, DEP_CLIENT  , "exists"             , COM_Exists_f				, ""			)
CMD_DEF (COM_InitFilesystem, DEP_CLIENT  , "path"               , COM_Path_f				, ""			)


CMD_DEF (Cache_Init , DEP_HOST__  , "flush"                     , Cache_Flush				, ""			)
CMD_DEF (Memory_Init, DEP_HOST__  , "hunk_print"                , Hunk_Print_f				, ""			) //johnfitz


CMD_DEF (Sky_Init   , DEP_CLIENT  , "sky"                       , Sky_SkyCommand_f			, ""			)
CMD_DEF (CDAudio_Init,DEP_CLIENT  , "cd"                        , CD_f						, ""			) // Needs to be available
CMD_DEF (CDAudio_Init,DEP_CL_MP3  , "setmusic"                  , Set_Music_f				, ""			) // Needs to be available

#ifndef SERVER_ONLY
CMD_DEF (Utilities_Init, DEP_HOST__  , "pak"                    , Pak_Command_f				, ""			)
CMD_DEF (Utilities_Init, DEP_HOST__  , "zip"                    , Zip_Command_f				, ""			)
CMD_DEF (Utilities_Init, DEP_HOST__  , "dir"                    , Dir_Command_f				, ""			)
CMD_DEF (Utilities_Init, DEP_HOST__  , "install"                , Install_Command_f				, ""			)
CMD_DEF (Utilities_Init, DEP_HOST__  , "servefiles"             , ServeFile_Command_f				, ""			)
CMD_DEF (Utilities_Init, DEP_HOST__  , "shutdown"               , ServeFile_Shutdown_Command_f				, ""			)
CMD_DEF (Utilities_Init, DEP_HOST__  , "getfile"                , GetFile_Command_f				, ""			)
CMD_DEF (Utilities_Init, DEP_HOST__  , "uninstall"              , UnInstall_Command_f				, ""			)
//CMD_DEF (Utilities_Init, DEP_HOST__  , "opend"                    , OpenD_Command_f				, ""			)
//CMD_DEF (Utilities_Init, DEP_HOST__  , "saved"                    , SaveD_Command_f				, ""			)
#endif // ! SERVER_ONLY






#undef CMD_DEF

