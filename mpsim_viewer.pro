HOME=$$system(echo $HOME)

# If your graphics board supports compute shader
DEFINES += HAVE_COMP_SHADER

# To have access to all parameters
#DEFINES += EXPERT_MODE

# Show tool tips
#DEFINES += SHOW_TOOLTIPS


include( mpsim_viewer.pri )
