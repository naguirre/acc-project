idf_component_register(SRCS "main.c" "wifi.c" "robot.cpp"
    INCLUDE_DIRS "include" "."
    EMBED_FILES server/error.html server/favicon.ico server/test.css server/test.js server/root.html)   