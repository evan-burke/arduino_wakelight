#include <TimerOne.h>

// This example creates a PWM signal with 25 kHz carrier.
//
// Arduino's analogWrite() gives you PWM output, but no control over the
// carrier frequency.  The default frequency is low, typically 490 or
// 3920 Hz.  Sometimes you may need a faster carrier frequency.
//
// The specification for 4-wire PWM fans recommends a 25 kHz frequency
// and allows 21 to 28 kHz.  The default from analogWrite() might work
// with some fans, but to follow the specification we need 25 kHz.
//
// http://www.formfactors.org/developer/specs/REV1_2_Public.pdf
//
// Connect the PWM pin to the fan's control wire (usually blue).  The
// board's ground must be connected to the fan's ground, and the fan
// needs +12 volt power from the computer or a separate power supply.

const int warmPin = 9;
const int coolPin = 10;

const int maxBright = 1023;
const int tempDiff = 768; // how far apart in brightness the two are when the cool starts turning on
const int totalBright = maxBright + tempDiff;
extern const uint16_t gamma[];

const float gammaVal = 2.4;

// loop vars
int bright = 0;
int warmBright = 0;
int coolBright = 0;
int warmReverse = 0;

void setup(void)
{
  Timer1.initialize(8000);  //
  Serial.begin(9600);
}

void loop(void)
{
  // slowly increase the brightness

  if ( warmReverse == 0 ) {
    warmBright = bright;  //increase by default
  }
  else {
    warmBright = warmBright - 1;  //decrease
  }
  if ( warmBright >= maxBright ) {
    if ( coolBright >= tempDiff && warmReverse == 0) {
      // only switch directions if warm is at max and cool is approaching max:
      warmReverse = 1;
      Serial.println("reversing");
      delay(500);
    } else {
      warmBright = maxBright;
    }
  }

  // calculate cool:
  if ( bright >= tempDiff ) {
    coolBright = bright - tempDiff;
  } else {
    coolBright = 0;
  }

  // calculate gamma. first convert to floats so arduino will do the math with the floats involved
  float wbtmp = warmBright;
  float cbtmp = coolBright;
  float mbtmp = maxBright;
  int warmGammaOut = (int)(pow(wbtmp/mbtmp, gammaVal) * maxBright);
  int coolGammaOut = (int)(pow(cbtmp/mbtmp, gammaVal) * maxBright);
 
  // set brigtnesses:
  //Timer1.pwm(warmPin, pgm_read_word(&gamma[warmBright]));
  //Timer1.pwm(coolPin, pgm_read_word(&gamma[coolBright]));

  Timer1.pwm(warmPin, warmGammaOut);
  Timer1.pwm(coolPin, coolGammaOut);


  int modulo = bright % 32;
  if ( modulo == 0 ) {
    delay(15);
    Serial.print("PWM Fan, Duty Cycle = ");
    Serial.print(bright);
        Serial.print("  warmBright: ");
        Serial.print(warmBright);
        Serial.print(" gO ");
        Serial.print(warmGammaOut);
    Serial.print("  warm gamma: ");
    Serial.print(pgm_read_word(&gamma[warmBright]));
    Serial.print("  cool gamma: ");
    Serial.println(pgm_read_word(&gamma[coolBright]));
  }

  // increase brightness
  bright = bright + 1;

  // minimum loop time:
  delay(5);
  // dynamic loop time, 1s per level at the lowest level, close to zero at the top
  int delayCurve = (int)( 2 * totalBright / (bright + 2));
  if (delayCurve >= 300) {
    delayCurve = 300;
  }
  //if (delayCurve >= 250) {
  //  Serial.print("Dynamic delay: ");
  //  Serial.println(delayCurve);
  //}
//  delay(delayCurve);


  // reset to zero
  if (bright >= totalBright) {
    delay(500);
    Serial.println("Resetting ");
    bright = 0;
    warmReverse = 0;
    Timer1.pwm(warmPin, bright);
    Timer1.pwm(coolPin, bright);
    delay(5000);
  }

}

// gamma 2.5
const uint16_t PROGMEM gamma[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,
  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9, 10, 10,
  10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12,
  13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15,
  16, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 18, 19, 19,
  19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 23,
  23, 23, 23, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 26, 27, 27,
  27, 28, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 31, 32,
  32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37,
  37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 42, 42, 42, 43,
  43, 43, 44, 44, 45, 45, 45, 46, 46, 46, 47, 47, 48, 48, 48, 49,
  49, 50, 50, 50, 51, 51, 52, 52, 53, 53, 53, 54, 54, 55, 55, 56,
  56, 56, 57, 57, 58, 58, 59, 59, 60, 60, 60, 61, 61, 62, 62, 63,
  63, 64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70, 71,
  71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79,
  79, 80, 80, 81, 82, 82, 83, 83, 84, 84, 85, 85, 86, 87, 87, 88,
  88, 89, 89, 90, 91, 91, 92, 92, 93, 94, 94, 95, 95, 96, 97, 97,
  98, 98, 99, 100, 100, 101, 102, 102, 103, 103, 104, 105, 105, 106, 107, 107,
  108, 109, 109, 110, 110, 111, 112, 112, 113, 114, 114, 115, 116, 117, 117, 118,
  119, 119, 120, 121, 121, 122, 123, 123, 124, 125, 126, 126, 127, 128, 128, 129,
  130, 131, 131, 132, 133, 133, 134, 135, 136, 136, 137, 138, 139, 139, 140, 141,
  142, 143, 143, 144, 145, 146, 146, 147, 148, 149, 149, 150, 151, 152, 153, 153,
  154, 155, 156, 157, 158, 158, 159, 160, 161, 162, 162, 163, 164, 165, 166, 167,
  167, 168, 169, 170, 171, 172, 173, 173, 174, 175, 176, 177, 178, 179, 180, 180,
  181, 182, 183, 184, 185, 186, 187, 188, 188, 189, 190, 191, 192, 193, 194, 195,
  196, 197, 198, 199, 200, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
  211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
  227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,
  243, 244, 245, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 260,
  261, 262, 263, 264, 265, 266, 267, 268, 270, 271, 272, 273, 274, 275, 276, 277,
  279, 280, 281, 282, 283, 284, 286, 287, 288, 289, 290, 291, 293, 294, 295, 296,
  297, 298, 300, 301, 302, 303, 304, 306, 307, 308, 309, 311, 312, 313, 314, 315,
  317, 318, 319, 320, 322, 323, 324, 325, 327, 328, 329, 330, 332, 333, 334, 336,
  337, 338, 339, 341, 342, 343, 345, 346, 347, 349, 350, 351, 352, 354, 355, 356,
  358, 359, 360, 362, 363, 364, 366, 367, 369, 370, 371, 373, 374, 375, 377, 378,
  379, 381, 382, 384, 385, 386, 388, 389, 391, 392, 393, 395, 396, 398, 399, 400,
  402, 403, 405, 406, 408, 409, 411, 412, 413, 415, 416, 418, 419, 421, 422, 424,
  425, 427, 428, 430, 431, 433, 434, 436, 437, 439, 440, 442, 443, 445, 446, 448,
  449, 451, 452, 454, 455, 457, 458, 460, 461, 463, 465, 466, 468, 469, 471, 472,
  474, 476, 477, 479, 480, 482, 483, 485, 487, 488, 490, 491, 493, 495, 496, 498,
  500, 501, 503, 504, 506, 508, 509, 511, 513, 514, 516, 518, 519, 521, 523, 524,
  526, 528, 529, 531, 533, 534, 536, 538, 540, 541, 543, 545, 546, 548, 550, 552,
  553, 555, 557, 558, 560, 562, 564, 565, 567, 569, 571, 572, 574, 576, 578, 580,
  581, 583, 585, 587, 588, 590, 592, 594, 596, 597, 599, 601, 603, 605, 607, 608,
  610, 612, 614, 616, 618, 619, 621, 623, 625, 627, 629, 631, 632, 634, 636, 638,
  640, 642, 644, 646, 648, 649, 651, 653, 655, 657, 659, 661, 663, 665, 667, 669,
  671, 673, 674, 676, 678, 680, 682, 684, 686, 688, 690, 692, 694, 696, 698, 700,
  702, 704, 706, 708, 710, 712, 714, 716, 718, 720, 722, 724, 726, 728, 730, 732,
  734, 736, 739, 741, 743, 745, 747, 749, 751, 753, 755, 757, 759, 761, 763, 766,
  768, 770, 772, 774, 776, 778, 780, 782, 785, 787, 789, 791, 793, 795, 797, 800,
  802, 804, 806, 808, 810, 813, 815, 817, 819, 821, 824, 826, 828, 830, 832, 835,
  837, 839, 841, 843, 846, 848, 850, 852, 855, 857, 859, 861, 864, 866, 868, 870,
  873, 875, 877, 880, 882, 884, 886, 889, 891, 893, 896, 898, 900, 903, 905, 907,
  910, 912, 914, 917, 919, 921, 924, 926, 928, 931, 933, 935, 938, 940, 942, 945,
  947, 950, 952, 954, 957, 959, 962, 964, 966, 969, 971, 974, 976, 979, 981, 983,
  986, 988, 991, 993, 996, 998, 1001, 1003, 1006,
  1008, 1011, 1013, 1016, 1018, 1021, 1023
};


