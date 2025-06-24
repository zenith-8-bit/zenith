/*
 * FluxGarage RoboEyes for OLED Displays V 1.0.1
 * Draws smoothly animated robot eyes on OLED displays, based on the Adafruit GFX 
 * library's graphics primitives, such as rounded rectangles and triangles.
 * * Copyright (C) 2024 Dennis Hoelscher
 * www.fluxgarage.com
 * www.youtube.com/@FluxGarage
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef _FLUXGARAGE_ROBOEYES_H
#define _FLUXGARAGE_ROBOEYES_H


// Usage of monochrome display colors
#define BGCOLOR 0 // background and overlays
#define MAINCOLOR 1 // drawings

// For mood type switch
#define DEFAULT 0
#define TIRED 1
#define ANGRY 2
#define HAPPY 3

// For turning things on or off
#define ON 1
#define OFF 0

// For switch "predefined positions"
#define N 1 // north, top center
#define NE 2 // north-east, top right
#define E 3 // east, middle right
#define SE 4 // south-east, bottom right
#define S 5 // south, bottom center
#define SW 6 // south-west, bottom left
#define W 7 // west, middle left
#define NW 8 // north-west, top left 
// for middle center set "DEFAULT"

class roboEyes
{
private:

// Yes, everything is currently still accessible. Be responsibly and don't mess things up :)

public:

// For general setup - screen size and max. frame rate
int screenWidth = 128; // OLED display width, in pixels
int screenHeight = 64; // OLED display height, in pixels
int frameInterval = 20; // default value for 50 frames per second (1000/50 = 20 milliseconds)
unsigned long fpsTimer = 0; // for timing the frames per second

// For controlling mood types and expressions
bool tired = 0;
bool angry = 0;
bool happy = 0;
bool curious = 0; // if true, draw the outer eye larger when looking left or right
bool cyclops = 0; // if true, draw only one eye
bool eyeL_open = 0; // left eye opened or closed?
bool eyeR_open = 0; // right eye opened or closed?


//*********************************************************************************************
//   Eyes Geometry
//*********************************************************************************************

// EYE LEFT - size and border radius
int eyeLwidthDefault = 36;
int eyeLheightDefault = 36;
int eyeLwidthCurrent = eyeLwidthDefault;
int eyeLheightCurrent = 1; // start with closed eye, otherwise set to eyeLheightDefault
int eyeLwidthNext = eyeLwidthDefault;
int eyeLheightNext = eyeLheightDefault;
int eyeLheightOffset = 0;
// Border Radius
byte eyeLborderRadiusDefault = 8;
byte eyeLborderRadiusCurrent = eyeLborderRadiusDefault;
byte eyeLborderRadiusNext = eyeLborderRadiusDefault;

// EYE RIGHT - size and border radius
int eyeRwidthDefault = eyeLwidthDefault;
int eyeRheightDefault = eyeLheightDefault;
int eyeRwidthCurrent = eyeRwidthDefault;
int eyeRheightCurrent = 1; // start with closed eye, otherwise set to eyeRheightDefault
int eyeRwidthNext = eyeRwidthDefault;
int eyeRheightNext = eyeRheightDefault;
int eyeRheightOffset = 0;
// Border Radius
byte eyeRborderRadiusDefault = 8;
byte eyeRborderRadiusCurrent = eyeRborderRadiusDefault;
byte eyeRborderRadiusNext = eyeRborderRadiusDefault;

// EYE LEFT - Coordinates
int eyeLxDefault = ((screenWidth)-(eyeLwidthDefault+spaceBetweenDefault+eyeRwidthDefault))/2;
int eyeLyDefault = ((screenHeight-eyeLheightDefault)/2);
int eyeLx = eyeLxDefault;
int eyeLy = eyeLyDefault;
int eyeLxNext = eyeLxDefault; // Initialize with default
int eyeLyNext = eyeLyDefault; // Initialize with default

// EYE RIGHT - Coordinates
int eyeRxDefault = eyeLx+eyeLwidthCurrent+spaceBetweenDefault;
int eyeRyDefault = eyeLy;
int eyeRx = eyeRxDefault;
int eyeRy = eyeRyDefault;
int eyeRxNext = eyeRxDefault; // Initialize with default
int eyeRyNext = eyeRyDefault; // Initialize with default

// BOTH EYES 
// Eyelid top size
byte eyelidsHeightMax = eyeLheightDefault/2; // top eyelids max height
byte eyelidsTiredHeight = 0;
byte eyelidsTiredHeightNext = eyelidsTiredHeight;
byte eyelidsAngryHeight = 0;
byte eyelidsAngryHeightNext = eyelidsAngryHeight;
// Bottom happy eyelids offset
byte eyelidsHappyBottomOffsetMax = (eyeLheightDefault/2)+3;
byte eyelidsHappyBottomOffset = 0;
byte eyelidsHappyBottomOffsetNext = 0;
// Space between eyes
int spaceBetweenDefault = 10;
int spaceBetweenCurrent = spaceBetweenDefault;
int spaceBetweenNext = 10;


//*********************************************************************************************
//   Macro Animations
//*********************************************************************************************

// Animation - horizontal flicker/shiver
bool hFlicker = 0;
bool hFlickerAlternate = 0;
byte hFlickerAmplitude = 2;

// Animation - vertical flicker/shiver
bool vFlicker = 0;
bool vFlickerAlternate = 0;
byte vFlickerAmplitude = 10;

// Animation - auto blinking
bool autoblinker = 0; // activate auto blink animation
int blinkInterval = 1; // basic interval between each blink in full seconds
int blinkIntervalVariation = 4; // interval variaton range in full seconds, random number inside of given range will be add to the basic blinkInterval, set to 0 for no variation
unsigned long blinktimer = 0; // for organising eyeblink timing

// Animation - idle mode: eyes looking in random directions
bool idle = 0;
int idleInterval = 1; // basic interval between each eye repositioning in full seconds
int idleIntervalVariation = 3; // interval variaton range in full seconds, random number inside of given range will be add to the basic idleInterval, set to 0 for no variation
unsigned long idleAnimationTimer = 0; // for organising eyeblink timing

// Animation - eyes confused: eyes shaking left and right
bool confused = 0;
unsigned long confusedAnimationTimer = 0;
int confusedAnimationDuration = 500;
bool confusedToggle = 1;

// Animation - eyes laughing: eyes shaking up and down
bool laugh = 0;
unsigned long laughAnimationTimer = 0;
int laughAnimationDuration = 500;
bool laughToggle = 1;


//*********************************************************************************************
//   GENERAL METHODS
//*********************************************************************************************

// Startup RoboEyes with defined screen-width, screen-height and max. frames per second
void begin(int width, int height, byte frameRate) {
  screenWidth = width; // OLED display width, in pixels
  screenHeight = height; // OLED display height, in pixels
  // Assuming 'display' object is available in the scope where this class is used
  // If 'display' is a member of this class, it would be 'this->display.clearDisplay();'
  // Or if 'display' is passed as a parameter to begin(), e.g., begin(..., Adafruit_SSD1306& disp)
  // For now, assuming it's globally accessible as per common Arduino examples.
  display.clearDisplay(); // clear the display buffer
  display.display(); // show empty screen
  eyeLheightCurrent = 1; // start with closed eyes
  eyeRheightCurrent = 1; // start with closed eyes
  setFramerate(frameRate); // calculate frame interval based on defined frameRate
}

void update(){
  // Limit drawing updates to defined max framerate
  if(millis()-fpsTimer >= frameInterval){
    drawEyes();
    fpsTimer = millis();
  }
}


//*********************************************************************************************
//   SETTERS METHODS
//*********************************************************************************************

// Calculate frame interval based on defined frameRate
void setFramerate(byte fps){
  frameInterval = 1000/fps;
}

void setWidth(byte leftEye, byte rightEye) {
  eyeLwidthNext = leftEye;
  eyeRwidthNext = rightEye;
  eyeLwidthDefault = leftEye;
  eyeRwidthDefault = rightEye;
}

void setHeight(byte leftEye, byte rightEye) {
  eyeLheightNext = leftEye;
  eyeRheightNext = rightEye;
  eyeLheightDefault = leftEye;
  eyeRheightDefault = rightEye;
}

// Set border radius for left and right eye
void setBorderradius(byte leftEye, byte rightEye) {
  eyeLborderRadiusNext = leftEye;
  eyeRborderRadiusNext = rightEye;
  eyeLborderRadiusDefault = leftEye;
  eyeRborderRadiusDefault = rightEye;
}

// Set space between the eyes, can also be negative
void setSpacebetween(int space) {
  spaceBetweenNext = space;
  spaceBetweenDefault = space;
}

// Set mood expression
void setMood(unsigned char mood)
  {
    switch (mood)
    {
    case TIRED:
      tired=1; 
      angry=0; 
      happy=0;
      break;
    case ANGRY:
      tired=0; 
      angry=1; 
      happy=0;
      break;
    case HAPPY:
      tired=0; 
      angry=0; 
      happy=1;
      break;
    default:
      tired=0; 
      angry=0; 
      happy=0;
      break;
    }
  }

// Set predefined position
void setPosition(unsigned char position)
  {
    switch (position)
    {
    case N:
      // North, top center
      eyeLxNext = getScreenConstraint_X()/4;
      eyeLyNext = 0;
      break;
    case NE:
      // North-east, top right
      eyeLxNext = getScreenConstraint_X()/2;
      eyeLyNext = 0;
      break;
    case E:
      // East, middle right
      eyeLxNext = getScreenConstraint_X()/2;
      eyeLyNext = getScreenConstraint_Y()/4;
      break;
    case SE:
      // South-east, bottom right
      eyeLxNext = getScreenConstraint_X()/2;
      eyeLyNext = getScreenConstraint_Y()/2;
      break;
    case S:
      // South, bottom center
      eyeLxNext = getScreenConstraint_X()/4;
      eyeLyNext = getScreenConstraint_Y()/2;
      break;
    case SW:
      // South-west, bottom left
      eyeLxNext = 0;
      eyeLyNext = getScreenConstraint_Y()/2;
      break;
    case W:
      // West, middle left
      eyeLxNext = 0;
      eyeLyNext = getScreenConstraint_Y()/4;
      break;
    case NW:
      // North-west, top left
      eyeLxNext = 0;
      eyeLyNext = 0;
      break;
    default:
      // Middle center
      eyeLxNext = getScreenConstraint_X()/4;
      eyeLyNext = getScreenConstraint_Y()/4;
      break;
    }
  }

// Set automated eye blinking, minimal blink interval in full seconds and blink interval variation range in full seconds
void setAutoblinker(bool active, int interval, int variation){
  autoblinker = active;
  blinkInterval = interval;
  blinkIntervalVariation = variation;
}
void setAutoblinker(bool active){
  autoblinker = active;
}

// Set idle mode - automated eye repositioning, minimal time interval in full seconds and time interval variation range in full seconds
void setIdleMode(bool active, int interval, int variation){
  idle = active;
  idleInterval = interval;
  idleIntervalVariation = variation;
}
void setIdleMode(bool active) {
  idle = active;
}

// Set curious mode - the respectively outer eye gets larger when looking left or right
void setCuriosity(bool curiousBit) {
  curious = curiousBit;
}

// Set cyclops mode - show only one eye 
void setCyclops(bool cyclopsBit) {
  cyclops = cyclopsBit;
}

// Set horizontal flickering (displacing eyes left/right)
void setHFlicker (bool flickerBit, byte Amplitude) {
  hFlicker = flickerBit; // turn flicker on or off
  hFlickerAmplitude = Amplitude; // define amplitude of flickering in pixels
}
void setHFlicker (bool flickerBit) {
  hFlicker = flickerBit; // turn flicker on or off
}


// Set vertical flickering (displacing eyes up/down)
void setVFlicker (bool flickerBit, byte Amplitude) {
  vFlicker = flickerBit; // turn flicker on or off
  vFlickerAmplitude = Amplitude; // define amplitude of flickering in pixels
}
void setVFlicker (bool flickerBit) {
  vFlicker = flickerBit; // turn flicker on or off
}


//*********************************************************************************************
//   GETTERS METHODS
//*********************************************************************************************

// Returns the max x position for left eye
int getScreenConstraint_X(){
  // Use default widths and space for constraint calculation to avoid flickering boundaries
  return screenWidth - eyeLwidthDefault - spaceBetweenDefault - eyeRwidthDefault;
} 

// Returns the max y position for left eye
int getScreenConstraint_Y(){
 return screenHeight - eyeLheightDefault; // using default height here, because height will vary when blinking and in curious mode
}


//*********************************************************************************************
//   BASIC ANIMATION METHODS
//*********************************************************************************************

// BLINKING FOR BOTH EYES AT ONCE
// Close both eyes
void close() {
  eyeLheightNext = 1; // closing left eye
  eyeRheightNext = 1; // closing right eye
  eyeL_open = 0; // left eye not opened (=closed)
  eyeR_open = 0; // right eye not opened (=closed)
}

// Open both eyes
void open() {
  eyeL_open = 1; // left eye opened - if true, drawEyes() will take care of opening eyes again
  eyeR_open = 1; // right eye opened
}

// Trigger eyeblink animation
void blink() {
  close();
  open();
}

// BLINKING FOR SINGLE EYES, CONTROL EACH EYE SEPARATELY
// Close eye(s)
void close(bool left, bool right) {
  if(left){
    eyeLheightNext = 1; // blinking left eye
    eyeL_open = 0; // left eye not opened (=closed)
  }
  if(right){
      eyeRheightNext = 1; // blinking right eye
      eyeR_open = 0; // right eye not opened (=closed)
  }
}

// Open eye(s)
void open(bool left, bool right) {
  if(left){
    eyeL_open = 1; // left eye opened - if true, drawEyes() will take care of opening eyes again
  }
  if(right){
    eyeR_open = 1; // right eye opened
  }
}

// Trigger eyeblink(s) animation
void blink(bool left, bool right) {
  close(left, right);
  open(left, right);
}


//*********************************************************************************************
//   MACRO ANIMATION METHODS
//*********************************************************************************************

// Play confused animation - one shot animation of eyes shaking left and right
void anim_confused() {
  confused = 1;
}

// Play laugh animation - one shot animation of eyes shaking up and down
void anim_laugh() {
  laugh = 1;
}

//*********************************************************************************************
//   PRE-CALCULATIONS AND ACTUAL DRAWINGS
//*********************************************************************************************

void drawEyes(){

  //// PRE-CALCULATIONS - EYE SIZES AND VALUES FOR ANIMATION TWEENINGS ////

  // Vertical size offset for larger eyes when looking left or right (curious gaze)
  if(curious){
    // Left eye curious logic
    if(eyeLxNext <= 10 && eyeLxNext < (getScreenConstraint_X()/2)) { // Only apply if truly looking left
      eyeLheightOffset = 6;
    } else if (cyclops && eyeLxNext >= (getScreenConstraint_X() - 10) && eyeLxNext > (getScreenConstraint_X()/2)) { // Cyclops looking right
      eyeLheightOffset = 6;
    } else {
      eyeLheightOffset = 0;
    }
    
    // Right eye curious logic (only if not cyclops)
    if (!cyclops) {
      if(eyeRxNext >= screenWidth - eyeRwidthDefault - 10 && eyeRxNext > (screenWidth / 2)) { // Only apply if truly looking right
        eyeRheightOffset = 6;
      } else {
        eyeRheightOffset = 0;
      }
    } else {
      eyeRheightOffset = 0; // No curious offset for right eye in cyclops mode
    }
  } else {
    eyeLheightOffset=0; // reset height offset for left eye
    eyeRheightOffset=0; // reset height offset for right eye
  }

  // Eye heights
  eyeLheightCurrent = (eyeLheightCurrent + eyeLheightNext + eyeLheightOffset)/2;
  eyeRheightCurrent = (eyeRheightCurrent + eyeRheightNext + eyeRheightOffset)/2;

  // Open eyes again after closing them
  if(eyeL_open){
    if(eyeLheightCurrent <= 1 + eyeLheightOffset){eyeLheightNext = eyeLheightDefault;} 
  }
  if(eyeR_open){
    if(eyeRheightCurrent <= 1 + eyeRheightOffset){eyeRheightNext = eyeRheightDefault;} 
  }

  // BLINKING SQUASH & STRETCH
  // Left Eye
  if (eyeLheightNext == 1) { // Eyes are closing (squash vertically, stretch horizontally)
    eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault + (eyeLheightDefault - eyeLheightCurrent) / 4)) / 2; 
  } else if (eyeLheightCurrent < eyeLheightDefault && eyeLheightNext == eyeLheightDefault) { // Eyes are opening (stretch vertically, slight squash horizontally)
    eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault - (eyeLheightDefault - eyeLheightCurrent) / 8)) / 2;
  } else {
    eyeLwidthCurrent = (eyeLwidthCurrent + eyeLwidthNext) / 2; // Revert to next (default) width
  }

  // Right Eye (if not cyclops)
  if (!cyclops) {
    if (eyeRheightNext == 1) { // Eyes are closing (squash vertically, stretch horizontally)
      eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault + (eyeRheightDefault - eyeRheightCurrent) / 4)) / 2;
    } else if (eyeRheightCurrent < eyeRheightDefault && eyeRheightNext == eyeRheightDefault) { // Eyes are opening (stretch vertically, slight squash horizontally)
      eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault - (eyeRheightDefault - eyeRheightCurrent) / 8)) / 2;
    } else {
      eyeRwidthCurrent = (eyeRwidthCurrent + eyeRwidthNext) / 2; // Revert to next (default) width
    }
  } else {
    eyeRwidthCurrent = 0; // Ensure right eye is fully closed in cyclops mode
  }
  

  // CURIOUS MODE SQUASH & STRETCH (additional to the default height offset)
  if (curious) {
    // When looking left (left eye becomes taller/narrower, right eye wider)
    if (eyeLxNext <= 10 && eyeLxNext < (getScreenConstraint_X()/2)) { 
      eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault - eyeLheightOffset / 2)) / 2; // Left eye: vertically stretched, horizontally squashed
      if (!cyclops) {
        eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault + eyeLheightOffset / 2)) / 2; // Right eye: horizontally stretched
      }
    }
    // When looking right (right eye becomes taller/narrower, left eye wider)
    else if (!cyclops && eyeRxNext >= screenWidth - eyeRwidthDefault - 10 && eyeRxNext > (screenWidth / 2)) { 
      eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault - eyeRheightOffset / 2)) / 2; // Right eye: vertically stretched, horizontally squashed
      eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault + eyeRheightOffset / 2)) / 2; // Left eye: horizontally stretched
    } else if (cyclops && eyeLxNext >= (getScreenConstraint_X() - 10) && eyeLxNext > (getScreenConstraint_X()/2)) { // Cyclops looking right
        eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault - eyeLheightOffset / 2)) / 2; // Left eye: vertically stretched, horizontally squashed
    }
    // If not in extreme left/right, smoothly transition back to default width
    else {
      eyeLwidthCurrent = (eyeLwidthCurrent + eyeLwidthNext) / 2;
      eyeRwidthCurrent = (eyeRwidthCurrent + eyeRwidthNext) / 2;
    }
  } else {
    // Ensure widths revert to default if curious is off
    eyeLwidthCurrent = (eyeLwidthCurrent + eyeLwidthNext) / 2;
    eyeRwidthCurrent = (eyeRwidthCurrent + eyeRwidthNext) / 2;
  }


  // Space between eyes
  spaceBetweenCurrent = (spaceBetweenCurrent + spaceBetweenNext)/2;

  // *** FIX START: Corrected Eye Coordinates Smoothing ***
  // These lines now simply smooth the current position towards the target 'Next' position
  // The 'Next' positions are set by setPosition() and Idle Mode.
  eyeLx = (eyeLx + eyeLxNext) / 2;
  eyeLy = (eyeLy + eyeLyNext) / 2;

  // Right eye's x position depends on left eye's position + the space between
  eyeRxNext = eyeLxNext + eyeLwidthCurrent + spaceBetweenCurrent; 
  eyeRyNext = eyeLyNext; // right eye's y position should be the same as for the left eye
  
  eyeRx = (eyeRx + eyeRxNext) / 2;
  eyeRy = (eyeRy + eyeRyNext) / 2;
  // *** FIX END ***


  // Left eye border radius
  eyeLborderRadiusCurrent = (eyeLborderRadiusCurrent + eyeLborderRadiusNext)/2;
  // Right eye border radius
  eyeRborderRadiusCurrent = (eyeRborderRadiusCurrent + eyeRborderRadiusNext)/2;
  

  //// APPLYING MACRO ANIMATIONS ////

  if(autoblinker){
    if(millis() >= blinktimer){
    blink();
    blinktimer = millis()+(blinkInterval*1000)+(random(blinkIntervalVariation)*1000); // calculate next time for blinking
    }
  }

  // Laughing - eyes shaking up and down for the duration defined by laughAnimationDuration (default = 500ms)
  if(laugh){
    if(laughToggle){
      setVFlicker(1, 5); // Activate vertical flicker for laugh
      laughAnimationTimer = millis();
      laughToggle = 0;
    } else if(millis() >= laughAnimationTimer+laughAnimationDuration){
      setVFlicker(0, 0); // Deactivate vertical flicker
      laughToggle = 1;
      laugh=0; 
    }
  }

  // Confused - eyes shaking left and right for the duration defined by confusedAnimationDuration (default = 500ms)
  if(confused){
    if(confusedToggle){
      setHFlicker(1, 10); // Activate horizontal flicker for confused
      confusedAnimationTimer = millis();
      confusedToggle = 0;
    } else if(millis() >= confusedAnimationTimer+confusedAnimationDuration){
      setHFlicker(0, 0); // Deactivate horizontal flicker
      confusedToggle = 1;
      confused=0; 
    }
  }

  // Idle - eyes moving to random positions on screen
  if(idle){
    if(millis() >= idleAnimationTimer){
      // Store current positions before calculating new random ones for potential stretch/squash based on movement
      int prevEyeLx = eyeLxNext; // Use eyeLxNext as previous target for direction
      int prevEyeLy = eyeLyNext; // Use eyeLyNext as previous target for direction
      
      eyeLxNext = random(getScreenConstraint_X() + 1); // +1 to ensure random reaches max boundary
      eyeLyNext = random(getScreenConstraint_Y() + 1); // +1 to ensure random reaches max boundary
      
      // Clamp eyeLxNext/eyeLyNext to within screen constraints
      eyeLxNext = constrain(eyeLxNext, 0, getScreenConstraint_X());
      eyeLyNext = constrain(eyeLyNext, 0, getScreenConstraint_Y());

      // A very subtle squash/stretch for idle movement (optional, can be removed if too much)
      int dx = abs(eyeLxNext - prevEyeLx);
      int dy = abs(eyeLyNext - prevEyeLy);
      
      // Only apply stretch/squash if there's significant movement
      if (dx > dy && dx > 2) { // More horizontal movement
          eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault + dx / 20)) / 2; // Slight horizontal stretch
          eyeLheightCurrent = (eyeLheightCurrent + (eyeLheightDefault - dx / 40)) / 2; // Slight vertical squash
          if (!cyclops) {
              eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault + dx / 20)) / 2;
              eyeRheightCurrent = (eyeRheightCurrent + (eyeRheightDefault - dx / 40)) / 2;
          }
      } else if (dy > dx && dy > 2) { // More vertical movement
          eyeLheightCurrent = (eyeLheightCurrent + (eyeLheightDefault + dy / 20)) / 2; // Slight vertical stretch
          eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault - dy / 40)) / 2; // Slight horizontal squash
          if (!cyclops) {
              eyeRheightCurrent = (eyeRheightCurrent + (eyeRheightDefault + dy / 20)) / 2;
              eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault - dy / 40)) / 2;
          }
      } else { // If movement is small or mostly diagonal, ease back to default size
          eyeLwidthCurrent = (eyeLwidthCurrent + eyeLwidthDefault) / 2;
          eyeLheightCurrent = (eyeLheightCurrent + eyeLheightDefault) / 2;
          if (!cyclops) {
            eyeRwidthCurrent = (eyeRwidthCurrent + eyeRwidthDefault) / 2;
            eyeRheightCurrent = (eyeRheightCurrent + eyeRheightDefault) / 2;
          }
      }
      
      idleAnimationTimer = millis()+(idleInterval*1000)+(random(idleIntervalVariation)*1000); // calculate next time for eyes repositioning
    }
  }


  // Adding offsets for horizontal flickering/shivering (with squash and stretch)
  if(hFlicker){
    if(hFlickerAlternate) {
      eyeLx += hFlickerAmplitude;
      eyeRx += hFlickerAmplitude;
      // Apply squash-stretch for horizontal movement
      eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault - hFlickerAmplitude)) / 2;
      eyeLheightCurrent = (eyeLheightCurrent + (eyeLheightDefault + hFlickerAmplitude / 2)) / 2;
      if (!cyclops) {
        eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault - hFlickerAmplitude)) / 2;
        eyeRheightCurrent = (eyeRheightCurrent + (eyeRheightDefault + hFlickerAmplitude / 2)) / 2;
      }
    } else {
      eyeLx -= hFlickerAmplitude;
      eyeRx -= hFlickerAmplitude;
      // Apply squash-stretch for horizontal movement
      eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault - hFlickerAmplitude)) / 2;
      eyeLheightCurrent = (eyeLheightCurrent + (eyeLheightDefault + hFlickerAmplitude / 2)) / 2;
      if (!cyclops) {
        eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault - hFlickerAmplitude)) / 2;
        eyeRheightCurrent = (eyeRheightCurrent + (eyeRheightDefault + hFlickerAmplitude / 2)) / 2;
      }
    }
    hFlickerAlternate = !hFlickerAlternate;
  }

  // Adding offsets for vertical flickering/shivering (with squash and stretch)
  if(vFlicker){
    if(vFlickerAlternate) {
      eyeLy += vFlickerAmplitude;
      eyeRy += vFlickerAmplitude;
      // Apply squash-stretch for vertical movement
      eyeLheightCurrent = (eyeLheightCurrent + (eyeLheightDefault + vFlickerAmplitude)) / 2;
      eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault - vFlickerAmplitude / 2)) / 2;
      if (!cyclops) {
        eyeRheightCurrent = (eyeRheightCurrent + (eyeRheightDefault + vFlickerAmplitude)) / 2;
        eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault - vFlickerAmplitude / 2)) / 2;
      }
    } else {
      eyeLy -= vFlickerAmplitude;
      eyeRy -= vFlickerAmplitude;
      // Apply squash-stretch for vertical movement
      eyeLheightCurrent = (eyeLheightCurrent + (eyeLheightDefault + vFlickerAmplitude)) / 2;
      eyeLwidthCurrent = (eyeLwidthCurrent + (eyeLwidthDefault - vFlickerAmplitude / 2)) / 2;
      if (!cyclops) {
        eyeRheightCurrent = (eyeRheightCurrent + (eyeRheightDefault + vFlickerAmplitude)) / 2;
        eyeRwidthCurrent = (eyeRwidthCurrent + (eyeRwidthDefault - vFlickerAmplitude / 2)) / 2;
      }
    }
    vFlickerAlternate = !vFlickerAlternate;
  }

  // Recalculate eye positions for centering after size changes (must be done near the end of pre-calculations)
  // These adjust the eyeLx/y and eyeRx/y *before* the final draw to keep them centered
  eyeLx += (eyeLwidthDefault - eyeLwidthCurrent) / 2;
  eyeLy += (eyeLheightDefault - eyeLheightCurrent) / 2;
  if (!cyclops) {
    eyeRx += (eyeRwidthDefault - eyeRwidthCurrent) / 2;
    eyeRy += (eyeRheightDefault - eyeRheightCurrent) / 2;
  }


  // Cyclops mode, set second eye's size and space between to 0
  if(cyclops){
    eyeRwidthCurrent = 0;
    eyeRheightCurrent = 0;
    spaceBetweenCurrent = 0;
  }


  //// ACTUAL DRAWINGS ////

  display.clearDisplay(); // start with a blank screen

  // Draw basic eye rectangles
  display.fillRoundRect(eyeLx, eyeLy, eyeLwidthCurrent, eyeLheightCurrent, eyeLborderRadiusCurrent, MAINCOLOR); // left eye
  if (!cyclops){
    display.fillRoundRect(eyeRx, eyeRy, eyeRwidthCurrent, eyeRheightCurrent, eyeRborderRadiusCurrent, MAINCOLOR); // right eye
  }

  // Prepare mood type transitions
  if (tired){eyelidsTiredHeightNext = eyeLheightCurrent/2; eyelidsAngryHeightNext = 0;} else{eyelidsTiredHeightNext = 0;}
  if (angry){eyelidsAngryHeightNext = eyeLheightCurrent/2; eyelidsTiredHeightNext = 0;} else{eyelidsAngryHeightNext = 0;}
  if (happy){eyelidsHappyBottomOffsetNext = eyeLheightCurrent/2;} else{eyelidsHappyBottomOffsetNext = 0;}

  // Draw tired top eyelids 
    eyelidsTiredHeight = (eyelidsTiredHeight + eyelidsTiredHeightNext)/2;
    if (!cyclops){
      display.fillTriangle(eyeLx, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx, eyeLy+eyelidsTiredHeight-1, BGCOLOR); // left eye 
      display.fillTriangle(eyeRx, eyeRy-1, eyeRx+eyeRwidthCurrent, eyeRy-1, eyeRx+eyeRwidthCurrent, eyeRy+eyelidsTiredHeight-1, BGCOLOR); // right eye
    } else {
      // Cyclops tired eyelids
      display.fillTriangle(eyeLx, eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx, eyeLy+eyelidsTiredHeight-1, BGCOLOR); // left eyelid half
      display.fillTriangle(eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy+eyelidsTiredHeight-1, BGCOLOR); // right eyelid half
    }

  // Draw angry top eyelids 
    eyelidsAngryHeight = (eyelidsAngryHeight + eyelidsAngryHeightNext)/2;
    if (!cyclops){ 
      display.fillTriangle(eyeLx, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy+eyelidsAngryHeight-1, BGCOLOR); // left eye
      display.fillTriangle(eyeRx, eyeRy-1, eyeRx+eyeRwidthCurrent, eyeRy-1, eyeRx, eyeRy+eyelidsAngryHeight-1, BGCOLOR); // right eye
    } else {
      // Cyclops angry eyelids
      display.fillTriangle(eyeLx, eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy+eyelidsAngryHeight-1, BGCOLOR); // left eyelid half
      display.fillTriangle(eyeLx+(eyeLwidthCurrent/2), eyeLy-1, eyeLx+eyeLwidthCurrent, eyeLy-1, eyeLx+(eyeLwidthCurrent/2), eyeLy+eyelidsAngryHeight-1, BGCOLOR); // right eyelid half
    }

  // Draw happy bottom eyelids
    eyelidsHappyBottomOffset = (eyelidsHappyBottomOffset + eyelidsHappyBottomOffsetNext)/2;
    // For happy, make sure it applies to the current eye height, not just default height
    display.fillRoundRect(eyeLx-1, (eyeLy+eyeLheightCurrent)-eyelidsHappyBottomOffset+1, eyeLwidthCurrent+2, eyeLheightCurrent+2, eyeLborderRadiusCurrent, BGCOLOR); // left eye
    if (!cyclops){ 
      display.fillRoundRect(eyeRx-1, (eyeRy+eyeRheightCurrent)-eyelidsHappyBottomOffset+1, eyeRwidthCurrent+2, eyeRheightCurrent+2, eyeRborderRadiusCurrent, BGCOLOR); // right eye
    }

  display.display(); // show drawings on display

} // end of drawEyes method


}; // end of class roboEyes

#endif