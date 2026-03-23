A vibe coded music player written in EFL. Work in progress

Compile with:

gcc main.c ui_callbacks.c ui_populate.c ui_settings.c ui_setup.c l
ibrary.c playback.c scanner.c -o player \
  $(pkg-config --cflags --libs elementary emotion eio eina ecore) \
  -ltag_c


