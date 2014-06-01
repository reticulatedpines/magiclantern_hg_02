/** \file
 * Entry points into the firmware image.
 *
 * These are the functions that we can call from our tasks
 * in the Canon 1.1.1 firmware for the 70D.
 *
 */
/*
 * Copyright (C) 2012 Magic Lantern Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License , or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful ,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not , write to the
 * Free Software Foundation , Inc. ,
 * 51 Franklin Street , Fifth Floor ,
 * Boston , MA  02110-1301 , USA.
 */

#include <stub.h>

.text

// Some functions are copied to RAM at around FF0C0098; they have to be called from RAM...


/** Startup **/
NSTUB( ROMBASEADDR , firmware_entry )
NSTUB(0xFF0C1BA8,  cstart)
NSTUB(   0x71618,  bzero32)
NSTUB(    0x3168,  create_init_task)
NSTUB(   0x7A9BC,  post_isr_hook)
NSTUB(   0x7A9B8,  pre_isr_hook)
NSTUB(0xFF0C54CC,  init_task)
//NSTUB(   ???,  additional_version)

/** File I/O **/
NSTUB(0xFF34C5AC,  FIO_CloseFile)
NSTUB(0xFF34D5A4,  FIO_FindClose)
NSTUB(0xFF34D4C4,  FIO_FindNextEx)
NSTUB(0xFF34C35C,  FIO_ReadFile)
NSTUB(0xFF34C4DC,  FIO_SeekFile)
NSTUB(0xFF34C4FC,  FIO_WriteFile)
NSTUB(0xFF34CE08, _FIO_CreateDirectory)
NSTUB(0xFF34C208, _FIO_CreateFile)
NSTUB(0xFF34D3D0, _FIO_FindFirstEx)
NSTUB(0xFF34C728, _FIO_GetFileSize)
NSTUB(0xFF34C14C, _FIO_OpenFile)
NSTUB(0xFF34C2B4, _FIO_RemoveFile)
NSTUB(0xFF34CC40, _FIO_RenameFile)

/** GUI **/
NSTUB(   0x7AE14,  gui_main_struct)
NSTUB(   0x9DBD8,  gui_task_list)
NSTUB(0xFF0D9D38,  GUI_ChangeMode)
NSTUB(0xFF0D9B7C,  GUI_Control)
NSTUB(0xFF196950,  SetGUIRequestMode)
NSTUB(0xFF0DA220,  gui_init_end)
NSTUB(0xFF0D97AC,  gui_main_task)
NSTUB(0xFF1B35F8,  gui_massive_event_loop)
// NSTUB(    ??? ,  GUI_SetLvMode)                           /* present on 7D.203 , 500D.111 , 5D2.212 , 50D.109 */
// NSTUB(    ??? ,  GUI_SetMovieSize_a)                      /* present on 50D.109 */
// NSTUB(    ??? ,  GUI_SetMovieSize_b)                      /* present on 50D.109 */
// NSTUB(    ??? ,  Gui_SetSoundRecord)                      /* present on 6D.113 , 7D.203 , 5D2.212 */

/** ASIF **/
NSTUB(0xFF14169C,  PowerAudioOutput)
NSTUB(0xFF13F728,  SetAudioVolumeOut)
// NSTUB(    ??? ,  SetAudioVolumeIn)                        /* present on 6D.113 , 7D.203 */
//NSTUB(   , SetNextASIFADCBuffer)
//NSTUB(   , SetNextASIFDACBuffer)
//NSTUB(   , SetSamplingRate )
// NSTUB(    ??? ,  SoundDevActiveIn)                        /* present on 6D.113 , EOSM.202 , 650D.104 , 7D.203 , 700D.113 */
// NSTUB(    ??? ,  SoundDevShutDownIn)                      /* present on 6D.113 , EOSM.202 , 650D.104 , 7D.203 , 700D.113 */
//NSTUB(   , StartASIFDMAADC)
//NSTUB(   , StartASIFDMADAC)
// NSTUB(    ??? ,  StopASIFDMAADC)                          /* present on 6D.113 , EOSM.202 , 7D.203 */
//NSTUB(   , StopASIFDMADAC)
// NSTUB(    ???,  PowerMicAmp)                             /* present on EOSM.202, 650D.104, 700D.113 */

/** Audio **/
NSTUB(0xFFA196A8,  audio_thresholds)
NSTUB(0xFF11936C,  sounddev_active_in)                      /* This one now calls another function, maybe canon refactored the common code path */
NSTUB(0xFF118F5C,  sounddev_task)
// NSTUB(    ??? , _audio_ic_read)
// NSTUB(    ??? , _audio_ic_write)
// NSTUB(    ??? ,  sounddev)                                /* present on 1100D.105 , 6D.113 , EOSM.202 , 650D.104 , 7D.203 , 500D.111 , 700D.113 , 5D2.212 , 550D.109 , 60D.111 , 600D.102 */

/** Bitmap **/
NSTUB(   0xBC308,  LCD_Palette)                             // in InitializeBitmapDisplayDevice , right after 0xc0f14800
NSTUB(   0xBDB08,  bmp_vram_info)                           // look in SetBitmapVramAddress - as in 60D

/** CFN **/
NSTUB(0xFF6C92B0,  GetCFnData)
NSTUB(0xFF6C9398,  SetCFnData)
// NSTUB(    ??? ,  GUI_GetCFnForTab4)                       /* present on 7D.203 */
// NSTUB(    ??? ,  GUI_SetCFnForTab4)                       /* present on 7D.203 */

/** Camera interface? **/
NSTUB(0xC0220000,  camera_engine)

/** DMA **/
NSTUB(    0xD834,  dma_memcpy)

/** Debug messages **/
NSTUB(    0x6904,  DryosDebugMsg)
NSTUB(   0x7B868,  dm_names)
NSTUB(    0x6C70,  dm_set_store_level)

/** Dialog API **/
NSTUB(0xFF4E20E0,  dialog_redraw)
NSTUB(0xFF393CDC,  dialog_set_property_str)

/** Dialog handlers **/
NSTUB(0xFF6F1620,  ErrCardForLVApp_handler)
NSTUB(0xFF53E1A0,  ErrForCamera_handler)
NSTUB(0xFF55DCA0,  LiveViewApp_handler)
NSTUB( 0xF56042C,  LiveViewApp_handler_end)                 // start of next function
NSTUB(0xFF6EFDF8,  LiveViewLevelApp_handler)
NSTUB(0xFF7469F0,  LiveViewShutterApp_handler)              /* present on 6D.113 , EOSM.202 , 650D.104 , 700D.113 */
NSTUB(0xFF3BE1B0,  PlayMain_handler)                        // Double check
NSTUB(0xFF589940,  PlayMovieGuideApp_handler)
NSTUB(0xFF56B3C0,  ShootOlcApp_handler)
// NSTUB(    ???, HideUnaviFeedBack_maybe)
// NSTUB(    ???, LiveViewApp_handler_BL_JudgeBottomInfoDispTimerState)
// NSTUB(    ???, LiveViewWbApp_handler)
// NSTUB(    ???,  LiveViewWifiApp_handler)                 /* present on 6D.113 */
// NSTUB(    ???,  StartPlayProtectGuideApp)                /* present on 7D.203 , 5D2.212 */
// NSTUB(    ???,  StopPlayProtectGuideApp)                 /* present on 7D.203 , 5D2.212 */

/** Display **/
NSTUB(0xFF5058B0,  MirrorDisplay)
NSTUB(0xFF505910,  NormalDisplay)
NSTUB(0xFF5058E0,  ReverseDisplay)

/** EDMAC **/
NSTUB(   0x37FF4,  ConnectReadEDmac)
NSTUB(   0x37F30,  ConnectWriteEDmac)
NSTUB(   0x385D4,  RegisterEDmacAbortCBR)
NSTUB(   0x38540,  RegisterEDmacCompleteCBR)
NSTUB(   0x38668,  RegisterEDmacPopCBR)
NSTUB(   0x37DD0,  SetEDmac)
NSTUB(   0x3817C,  StartEDmac)
NSTUB(   0x38610,  UnregisterEDmacAbortCBR)
NSTUB(   0x3857C,  UnregisterEDmacCompleteCBR)
NSTUB(   0x386A4,  UnregisterEDmacPopCBR)

/** Electronic Level **/
NSTUB(0xFF27D010,  GUI_SetRollingPitchingLevelStatus)

/** Engio **/
NSTUB(0xFF2BC3AC, _EngDrvOut)                               // AJ_EngDrvOut_1xVar_to_ShadowStruct
NSTUB(0xFF2BC6C4, _engio_write)
NSTUB(0xFF2BC448,  shamem_read)                             // AJ_0x8FB0_engio_struct_n_R0_manipulation_to_get_ptr
// NSTUB(    ??? ,  EngDrvOut)                               /* present on 7D_MASTER.203 */

/** Eventprocs (call by name) **/
NSTUB(0xFF14439C,  call)

/** Events **/
NSTUB(   0x3DD24,  TryPostEvent)
NSTUB(   0x3DD80,  TryPostEvent_end)
NSTUB(   0x3D644,  TryPostStageEvent)
NSTUB(   0x3D6A0,  TryPostStageEvent_end)

/** ExMem **/
NSTUB(    0xC420,  CreateMemorySuite)
NSTUB(    0xC070,  DeleteMemorySuite)
NSTUB(    0xBD8C,  GetFirstChunkFromSuite)
NSTUB(    0xBA08,  GetMemoryAddressOfMemoryChunk)
NSTUB(    0xCA90,  GetNextMemoryChunk)
NSTUB(0xFF147F88,  AllocateContinuousMemoryResource)        // m_pfContAllocMemoryCBR
NSTUB(0xFF147F3C,  AllocateMemoryResource)                  // m_pfAllocMemoryCBR
NSTUB(0xFF1480A0,  FreeMemoryResource)                      // m_pfFreeMemoryCBR

/** GUI for old cameras **/
// NSTUB(    ??? ,  gui_change_lcd_state_post)               /* present on 7D.203 , 500D.111 , 5D2.212 , 50D.109 */
// NSTUB(    ??? ,  gui_change_mode)                         /* present on 1100D.105 , 7D.203 , 500D.111 , 5D2.212 , 50D.109 , 550D.109 , 600D.102 */
// NSTUB(    ??? ,  gui_change_shoot_type_post)              /* present on 7D.203 , 500D.111 , 5D2.212 , 50D.109 */
// NSTUB(    ??? ,  gui_init_event)                          /* present on 7D.203 , 500D.111 , 5D2.212 , 50D.109 */
// NSTUB(    ??? ,  gui_local_post)                          /* present on 1100D.105 , 7D.203 , 500D.111 , 5D2.212 , 50D.109 , 550D.109 , 600D.102 */
// NSTUB(    ??? ,  gui_other_post)                          /* present on 1100D.105 , 7D.203 , 500D.111 , 5D2.212 , 50D.109 , 550D.109 , 600D.102 */
// NSTUB(    ??? ,  gui_post_10000085)                       /* present on 7D.203 , 500D.111 , 5D2.212 , 50D.109 */
// NSTUB(    ??? ,  gui_timer_something)                     /* present on 7D.203 , 500D.111 , 5D2.212 , 50D.109 */
// NSTUB(    ??? ,  gui_timer_struct)                        /* present on 1100D.105 , 7D.203 , 500D.111 , 5D2.212 , 50D.109 , 550D.109 , 60D.111 , 600D.102 */

/** GUI timers **/
NSTUB(0xFF54A2D0,  CancelDateTimer)
NSTUB(    0xE8AC,  SetTimerAfter)
///NSTUB(    0xEAAC,  TimerCancel)

/** H264 Encoder **/
NSTUB(   0x946E0,  mvr_config)
NSTUB(0xFF2BB65C,  mvrFixQScale)                            // FIXME: use call()
NSTUB(0xFF2BB154,  mvrSetDefQScale)
NSTUB(0xFF2BB18C,  mvrSetFullHDOptSize)
NSTUB(0xFF2BB370,  mvrSetGopOptSizeFULLHD)

/** Interrupts **/
// NSTUB(    ??? ,  isr_table_handler)                       /* present on 7D_MASTER.203 */
// NSTUB(    ??? ,  isr_table_param)                         /* present on 7D_MASTER.203 */

/** LCD Sensor **/
// NSTUB(    ???,  DispSensorStart)                         /* present on 650D.104, 7D.203, 500D.111, 700D.113, 5D2.212, 50D.109, 550D.109, 600D.102 */
// NSTUB(    ???,  LightMeasure_n_Callback_r0)              /* present on 7D.203, 5D2.212 */

/** Making the card bootable **/
NSTUB(   0xA8128,  sd_device)
NSTUB(0xFF67CF84,  fsuDecodePartitionTable)                 // AJ_fsuGetPart_related
// NSTUB(   0x20890 , cf_device) idk...

/** Memory allocation **/
NSTUB(    0xA4FC,  GetMemoryInformation)
NSTUB(    0xA4A8,  GetSizeOfMaxRegion)
NSTUB(    0xA75C, _AllocateMemory)
NSTUB(    0xAAF8, _FreeMemory)
NSTUB(   0x3DF6C, _alloc_dma_memory)
NSTUB(   0x3DFA0, _free_dma_memory)
NSTUB(0xFF0C8E3C, _free)                                    // see "can't malloc"
NSTUB(0xFF0C8DE4, _malloc)
// NSTUB(    ??? ,  free_dma_memory)                         /* present on 7D_MASTER.203 */

/** Message queues **/
NSTUB(   0x3A824,  msg_queue_count)
NSTUB(   0x3A4E8,  msg_queue_create)
NSTUB(   0x3A7E4,  msg_queue_post)
NSTUB(   0x3A5F8,  msg_queue_receive)

/** PTP **/
NSTUB(0xFF27C868,  ptpPropSetUILock)
NSTUB(0xFF29F7BC,  ptp_register_handler)

/** Properties **/
NSTUB(0xFF12CB14,  PROPAD_GetPropertyData)
NSTUB(0xFF12B15C, _prop_cleanup)
NSTUB(0xFF12AB14, _prop_request_change)
NSTUB(0xFF12B090,  prop_deliver)
NSTUB(0xFF12A924,  prop_register_slave)
// NSTUB(    ??? ,  prop_unregister_slave)                   /* present on 7D.203 */

/** RPC (for 7D) **/
// NSTUB(    ??? ,  BulkInIPCTransfer)                       /* present on 7D.203 */
// NSTUB(    ??? ,  BulkOutIPCTransfer)                      /* present on 7D.203 */
// NSTUB(    ??? ,  RegisterRPCHandler)                      /* present on 7D.203 , 7D_MASTER.203 */
// NSTUB(    ??? ,  RequestRPC)                              /* present on 7D.203 , 7D_MASTER.203 */

/** Recursive locks **/
NSTUB(   0x3A930,  AcquireRecursiveLock)                    // AJ_KernelDry_KerRLock.c
NSTUB(    0xD4D0,  CreateRecursiveLock)
NSTUB(   0x3AA44,  ReleaseRecursiveLock)                    // AJ_KernelDry_KerRLock.c_p2

/** ResLock **/
NSTUB(0xFF2C049C,  CreateResLockEntry)
NSTUB(0xFF2C095C,  LockEngineResources)
NSTUB(0xFF2C0B00,  UnLockEngineResources)

/** Semaphores **/
NSTUB(    0x91DC,  create_named_semaphore)
NSTUB(    0x9428,  give_semaphore)
NSTUB(    0x933C,  take_semaphore)

/** Task info **/
NSTUB(    0x259C,  get_current_task)
NSTUB(   0x11DF4,  get_obj_attr)                            // AJ_checks_if_Process_id_created_by_Dryos
NSTUB(   0x51358,  is_taskid_valid)                         // AJ_task_trampoline_related_p10
NSTUB(   0x7BEF4,  task_max)

/** Task sleep **/
NSTUB(    0x9818,  msleep)

/** Tasks **/
NSTUB(    0x98CC,  task_create)
NSTUB(   0x7A9B4,  task_dispatch_hook)
NSTUB(    0xEF60,  task_trampoline)
// NSTUB(    ??? ,  current_task_ctx)                        /* present on 7D.203 */
// NSTUB(    ??? ,  next_task_ctx)                           /* present on 7D.203 */

/** Misc **/
NSTUB(   0x3BFC4,  vsnprintf)
NSTUB(0xFF13D2A0,  LoadCalendarFromRTC)
// NSTUB(    ??? ,  EnableImagePhysicalScreenParameter)      /* present on 7D.203 , 5D2.212 , 50D.109 */
// NSTUB(    ??? ,  PtpDps_remote_release_SW1_SW2_worker)    /* present on 7D.203 , 5D2.212 */
// NSTUB(    ??? ,  lv_path_struct)                          /* present on 1100D.105 , 7D.203 , 550D.109 , 60D.111 , 600D.102 */
// NSTUB(    ??? ,  memcpy)                                  /* present on 7D_MASTER.203 */
