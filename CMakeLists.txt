target_sources(app PRIVATE 
    src/behaviors/behavior_bt_morph.c 
    src/behaviors/behavior_smart_ctrl_alt.c
    src/behaviors/behavior_smart_alt_ctrl.c
    src/dual_key_layer.c
    src/events/mac_connection_changed.c
    src/mac_profile_monitor.c
    src/mac_layer_handler.c
    src/bt_profile_rgb.c
)

target_include_directories(app PRIVATE src)
