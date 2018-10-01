import pygame


pygame.init()
pygame.joystick.init()

joystick_count = pygame.joystick.get_count()
print('Nb joystcks : {0:d}'.format(joystick_count))

done = False
clock = pygame.time.Clock()
joystick = pygame.joystick.Joystick(0)
    
joystick.init()
name = joystick.get_name()
print("Joystick name: {}".format(name) )
    

# -------- Main Program Loop -----------
while done==False:
    # EVENT PROCESSING STEP
    for event in pygame.event.get(): # User did something
        if event.type == pygame.QUIT: # If user clicked close
            done=True # Flag that we are done so we exit this loop
        # Possible joystick actions: JOYAXISMOTION JOYBALLMOTION JOYBUTTONDOWN JOYBUTTONUP JOYHATMOTION
        if event.type == pygame.JOYBUTTONDOWN:
            print("Joystick button pressed.")
        if event.type == pygame.JOYBUTTONUP:
            print("Joystick button released.")
            

    # Get the name from the OS for the controller/joystick
    
    axes = joystick.get_numaxes()
        
    for i in range( axes ):
        axis = joystick.get_axis( i )
        print("Axis {} value: {:>6.3f}".format(i, axis) )
        
    buttons = joystick.get_numbuttons()
    
    for i in range( buttons ):
        button = joystick.get_button( i )
        print("Button {:>2} value: {}".format(i,button) )

    
    clock.tick(60)
