/*
 *
 * Author Vlad Seryakov vlad@crystalballinc.com
 *   
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://mozilla.org/.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 */

#include "lmbox.h"

typedef struct VideoNorms {
    char *name;
    int id[2];
    int width;
    int height;
};

typedef struct VideoFreqs {
    char *name;
    int freq;
    int freq2;
};

typedef struct VideoChannels {
    char *name;
    struct VideoFreqs *freqs;
};

/* US broadcast */
static VideoFreqs ntsc_bcast[] = {
    {"2", 55250, 0}, {"3", 61250, 0}, {"4", 67250, 0},
    {"5", 77250, 0}, {"6", 83250, 0}, {"7", 175250, 0},
    {"8", 181250, 0}, {"9", 187250, 0},
    {"10", 193250, 0}, {"11", 199250, 0}, {"12", 205250, 0},
    {"13", 211250, 0}, {"14", 471250, 0},
    {"15", 477250, 0}, {"16", 483250, 0}, {"17", 489250, 0},
    {"18", 495250, 0}, {"19", 501250, 0}, {"20", 507250, 0},
    {"21", 513250, 0}, {"22", 519250, 0},
    {"23", 525250, 0}, {"24", 531250, 0}, {"25", 537250, 0},
    {"26", 543250, 0}, {"27", 549250, 0}, {"28", 555250, 0},
    {"29", 561250, 0}, {"30", 567250, 0},
    {"31", 573250, 0}, {"32", 579250, 0}, {"33", 585250, 0},
    {"34", 591250, 0}, {"35", 597250, 0}, {"36", 603250, 0},
    {"37", 609250, 0}, {"38", 615250, 0},
    {"39", 621250, 0}, {"40", 627250, 0}, {"41", 633250, 0},
    {"42", 639250, 0}, {"43", 645250, 0}, {"44", 651250, 0},
    {"45", 657250, 0}, {"46", 663250, 0},
    {"47", 669250, 0}, {"48", 675250, 0}, {"49", 681250, 0},
    {"50", 687250, 0}, {"51", 693250, 0}, {"52", 699250, 0},
    {"53", 705250, 0}, {"54", 711250, 0},
    {"55", 717250, 0}, {"56", 723250, 0}, {"57", 729250, 0},
    {"58", 735250, 0}, {"59", 741250, 0}, {"60", 747250, 0},
    {"61", 753250, 0}, {"62", 759250, 0},
    {"63", 765250, 0}, {"64", 771250, 0}, {"65", 777250, 0},
    {"66", 783250, 0}, {"67", 789250, 0}, {"68", 795250, 0},
    {"69", 801250, 0}, {"70", 807250, 0},
    {"71", 813250, 0}, {"72", 819250, 0}, {"73", 825250, 0},
    {"74", 831250, 0}, {"75", 837250, 0}, {"76", 843250, 0},
    {"77", 849250, 0}, {"78", 855250, 0},
    {"79", 861250, 0}, {"80", 867250, 0}, {"81", 873250, 0},
    {"82", 879250, 0}, {"83", 885250, 0},
    {0, 0}
};

/* US cable */
static VideoFreqs ntsc_cable[] = {
    {"1", 73250, 0}, {"2", 55250, 0}, {"3", 61250, 0},
    {"4", 67250, 0}, {"5", 77250, 0}, {"6", 83250, 0},
    {"7", 175250, 0}, {"8", 181250, 0},
    {"9", 187250, 0}, {"10", 193250, 0}, {"11", 199250, 0},
    {"12", 205250, 0}, {"13", 211250, 0}, {"14", 121250, 0},
    {"15", 127250, 0}, {"16", 133250, 0},
    {"17", 139250, 0}, {"18", 145250, 0}, {"19", 151250, 0},
    {"20", 157250, 0}, {"21", 163250, 0}, {"22", 169250, 0},
    {"23", 217250, 0}, {"24", 223250, 0},
    {"25", 229250, 0}, {"26", 235250, 0}, {"27", 241250, 0},
    {"28", 247250, 0},
    {"29", 253250, 0}, {"30", 259250, 0}, {"31", 265250, 0},
    {"32", 271250, 0},
    {"33", 277250, 0}, {"34", 283250, 0}, {"35", 289250, 0},
    {"36", 295250, 0},
    {"37", 301250, 0}, {"38", 307250, 0}, {"39", 313250, 0},
    {"40", 319250, 0},
    {"41", 325250, 0}, {"42", 331250, 0}, {"43", 337250, 0},
    {"44", 343250, 0},
    {"45", 349250, 0}, {"46", 355250, 0}, {"47", 361250, 0},
    {"48", 367250, 0}, {"49", 373250, 0}, {"50", 379250, 0},
    {"51", 385250, 0}, {"52", 391250, 0}, {"53", 397250, 0},
    {"54", 403250, 0}, {"55", 409250, 0}, {"56", 415250, 0},
    {"57", 421250, 0}, {"58", 427250, 0}, {"59", 433250, 0},
    {"60", 439250, 0}, {"61", 445250, 0}, {"62", 451250, 0},
    {"63", 457250, 0}, {"64", 463250, 0}, {"65", 469250, 0},
    {"66", 475250, 0}, {"67", 481250, 0}, {"68", 487250, 0},
    {"69", 493250, 0}, {"70", 499250, 0}, {"71", 505250, 0},
    {"72", 511250, 0}, {"73", 517250, 0}, {"74", 523250, 0},
    {"75", 529250, 0}, {"76", 535250, 0}, {"77", 541250, 0},
    {"78", 547250, 0}, {"79", 553250, 0}, {"80", 559250, 0},
    {"81", 565250, 0}, {"82", 571250, 0}, {"83", 577250, 0},
    {"84", 583250, 0}, {"85", 589250, 0}, {"86", 595250, 0},
    {"87", 601250, 0}, {"88", 607250, 0}, {"89", 613250, 0},
    {"90", 619250, 0}, {"91", 625250, 0}, {"92", 631250, 0},
    {"93", 637250, 0}, {"94", 643250, 0}, {"95", 91250, 0},
    {"96", 97250, 0}, {"97", 103250, 0}, {"98", 109250, 0},
    {"99", 115250, 0}, {"100", 649250, 0}, {"101", 655250, 0},
    {"102", 661250, 0}, {"103", 667250, 0}, {"104", 673250, 0},
    {"105", 679250, 0}, {"106", 685250, 0}, {"107", 691250, 0},
    {"108", 697250, 0}, {"109", 703250, 0}, {"110", 709250, 0},
    {"111", 715250, 0}, {"112", 721250, 0}, {"113", 727250, 0},
    {"114", 733250, 0}, {"115", 739250, 0}, {"116", 745250, 0},
    {"117", 751250, 0}, {"118", 757250, 0}, {"119", 763250, 0},
    {"120", 769250, 0}, {"121", 775250, 0}, {"122", 781250, 0},
    {"123", 787250, 0}, {"124", 793250, 0}, {"125", 799250, 0},
    {"T7", 8250, 0}, {"T8", 14250, 0}, {"T9", 20250, 0},
    {"T10", 26250, 0}, {"T11", 32250, 0}, {"T12", 38250, 0},
    {"T13", 44250, 0}, {"T14", 50250, 0},
    {0, 0}
};

/* US HRC */
static VideoFreqs ntsc_hrc[] = {
    {"1", 72000, 0}, {"2", 54000, 0}, {"3", 60000, 0}, {"4", 66000, 0},
    {"5", 78000, 0}, {"6", 84000, 0}, {"7", 174000, 0}, {"8", 180000, 0},
    {"9", 186000, 0}, {"10", 192000, 0}, {"11", 198000, 0},
    {"12", 204000, 0}, {"13", 210000, 0}, {"14", 120000, 0},
    {"15", 126000, 0}, {"16", 132000, 0}, {"17", 138000, 0},
    {"18", 144000, 0}, {"19", 150000, 0}, {"20", 156000, 0},
    {"21", 162000, 0}, {"22", 168000, 0}, {"23", 216000, 0},
    {"24", 222000, 0}, {"25", 228000, 0}, {"26", 234000, 0},
    {"27", 240000, 0}, {"28", 246000, 0}, {"29", 252000, 0},
    {"30", 258000, 0}, {"31", 264000, 0}, {"32", 270000, 0},
    {"33", 276000, 0}, {"34", 282000, 0}, {"35", 288000, 0},
    {"36", 294000, 0}, {"37", 300000, 0}, {"38", 306000, 0},
    {"39", 312000, 0}, {"40", 318000, 0}, {"41", 324000, 0},
    {"42", 330000, 0}, {"43", 336000, 0}, {"44", 342000, 0},
    {"45", 348000, 0}, {"46", 354000, 0}, {"47", 360000, 0},
    {"48", 366000, 0}, {"49", 372000, 0}, {"50", 378000, 0},
    {"51", 384000, 0}, {"52", 390000, 0}, {"53", 396000, 0},
    {"54", 402000, 0}, {"55", 408000, 0}, {"56", 414000, 0},
    {"57", 420000, 0}, {"58", 426000, 0}, {"59", 432000, 0},
    {"60", 438000, 0}, {"61", 444000, 0}, {"62", 450000, 0},
    {"63", 456000, 0}, {"64", 462000, 0}, {"65", 468000, 0},
    {"66", 474000, 0}, {"67", 480000, 0}, {"68", 486000, 0},
    {"69", 492000, 0}, {"70", 498000, 0}, {"71", 504000, 0},
    {"72", 510000, 0}, {"73", 516000, 0}, {"74", 522000, 0},
    {"75", 528000, 0}, {"76", 534000, 0}, {"77", 540000, 0},
    {"78", 546000, 0}, {"79", 552000, 0}, {"80", 558000, 0},
    {"81", 564000, 0}, {"82", 570000, 0}, {"83", 576000, 0},
    {"84", 582000, 0}, {"85", 588000, 0}, {"86", 594000, 0},
    {"87", 600000, 0}, {"88", 606000, 0}, {"89", 612000, 0},
    {"90", 618000, 0}, {"91", 624000, 0}, {"92", 630000, 0},
    {"93", 636000, 0}, {"94", 642000, 0}, {"95", 900000, 0},
    {"96", 960000, 0}, {"97", 102000, 0}, {"98", 108000, 0},
    {"99", 114000, 0}, {"100", 648000, 0}, {"101", 654000, 0},
    {"102", 660000, 0}, {"103", 666000, 0}, {"104", 672000, 0},
    {"105", 678000, 0}, {"106", 684000, 0}, {"107", 690000, 0},
    {"108", 696000, 0}, {"109", 702000, 0}, {"110", 708000, 0},
    {"111", 714000, 0}, {"112", 720000, 0}, {"113", 726000, 0},
    {"114", 732000, 0}, {"115", 738000, 0}, {"116", 744000, 0},
    {"117", 750000, 0}, {"118", 756000, 0}, {"119", 762000, 0},
    {"120", 768000, 0}, {"121", 774000, 0}, {"122", 780000, 0},
    {"123", 786000, 0}, {"124", 792000, 0}, {"125", 798000, 0},
    {"T7", 7000, 0}, {"T8", 13000, 0}, {"T9", 19000, 0},
    {"T10", 25000, 0}, {"T11", 31000, 0}, {"T12", 37000, 0},
    {"T13", 43000, 0}, {"T14", 49000, 0},
    {0, 0}
};

/* Canada cable */
static VideoFreqs ntsc_cable_ca[] = {
    {"2", 61750, 0}, {"3", 67750, 0}, {"4", 73750, 0}, {"5", 83750, 0},
    {"6", 89750, 0}, {"7", 181750, 0}, {"8", 187750, 0}, {"9", 193750, 0},
    {"10", 199750, 0}, {"11", 205750, 0}, {"12", 211750, 0},
    {"13", 217750, 0}, {"14", 127750, 0}, {"15", 133750, 0},
    {"16", 139750, 0}, {"17", 145750, 0}, {"18", 151750, 0},
    {"19", 157750, 0}, {"20", 163750, 0}, {"21", 169750, 0},
    {"22", 175750, 0}, {"23", 223750, 0}, {"24", 229750, 0},
    {"25", 235750, 0}, {"26", 241750, 0}, {"27", 247750, 0},
    {"28", 253750, 0}, {"29", 259750, 0}, {"30", 265750, 0},
    {"31", 271750, 0}, {"32", 277750, 0}, {"33", 283750, 0},
    {"34", 289750, 0}, {"35", 295750, 0}, {"36", 301750, 0},
    {"37", 307750, 0}, {"38", 313750, 0}, {"39", 319750, 0},
    {"40", 325750, 0}, {"41", 331750, 0}, {"42", 337750, 0},
    {"43", 343750, 0}, {"44", 349750, 0}, {"45", 355750, 0},
    {"46", 361750, 0}, {"47", 367750, 0}, {"48", 373750, 0},
    {"49", 379750, 0}, {"50", 385750, 0}, {"51", 391750, 0},
    {"52", 397750, 0}, {"53", 403750, 0}, {"54", 409750, 0},
    {"55", 415750, 0}, {"56", 421750, 0}, {"57", 427750, 0},
    {"58", 433750, 0}, {"59", 439750, 0}, {"60", 445750, 0},
    {"61", 451750, 0}, {"62", 457750, 0}, {"63", 463750, 0},
    {"64", 469750, 0}, {"65", 475750, 0}, {"66", 481750, 0},
    {"67", 487750, 0}, {"68", 493750, 0}, {"69", 499750, 0},
    {"70", 505750, 0}, {"71", 511750, 0}, {"72", 517750, 0},
    {"73", 523750, 0}, {"74", 529750, 0}, {"75", 535750, 0},
    {"76", 541750, 0}, {"77", 547750, 0}, {"78", 553750, 0},
    {"79", 559750, 0}, {"80", 565750, 0}, {"81", 571750, 0},
    {"82", 577750, 0}, {"83", 583750, 0}, {"84", 589750, 0},
    {"85", 595750, 0}, {"86", 601750, 0}, {"87", 607750, 0},
    {"88", 613750, 0}, {"89", 619750, 0}, {"90", 625750, 0},
    {"91", 631750, 0}, {"92", 637750, 0}, {"93", 643750, 0},
    {"94", 649750, 0}, {"95", 97750, 0}, {"96", 103750, 0},
    {"97", 109750, 0}, {"98", 115750, 0}, {"99", 121750, 0},
    {"100", 655750, 0}, {"101", 661750, 0}, {"102", 667750, 0},
    {"103", 673750, 0}, {"104", 679750, 0}, {"105", 685750, 0},
    {"106", 691750, 0}, {"107", 697750, 0}, {"108", 703750, 0},
    {"109", 709750, 0}, {"110", 715750, 0}, {"111", 721750, 0},
    {"112", 727750, 0}, {"113", 733750, 0}, {"114", 739750, 0},
    {"115", 745750, 0}, {"116", 751750, 0}, {"117", 757750, 0},
    {"118", 763750, 0}, {"119", 769750, 0}, {"120", 775750, 0},
    {"121", 781750, 0}, {"122", 787750, 0}, {"123", 793750, 0},
    {"124", 799750, 0}, {"125", 805750, 0},
    {0, 0}
};

/* JP broadcast */
static VideoFreqs ntsc_bcast_jp[] = {
    {"1", 91250, 0}, {"2", 97250, 0}, {"3", 103250, 0}, {"4", 171250, 0},
    {"5", 177250, 0}, {"6", 183250, 0}, {"7", 189250, 0}, {"8", 193250, 0},
    {"9", 199250, 0}, {"10", 205250, 0}, {"11", 211250, 0}, {"12", 217250, 0},
    {"13", 471250, 0}, {"14", 477250, 0}, {"15", 483250, 0},
    {"16", 489250, 0}, {"17", 495250, 0}, {"18", 501250, 0},
    {"19", 507250, 0}, {"20", 513250, 0}, {"21", 519250, 0},
    {"22", 525250, 0}, {"23", 531250, 0}, {"24", 537250, 0},
    {"25", 543250, 0}, {"26", 549250, 0}, {"27", 555250, 0},
    {"28", 561250, 0}, {"29", 567250, 0}, {"30", 573250, 0},
    {"31", 579250, 0}, {"32", 585250, 0}, {"33", 591250, 0},
    {"34", 597250, 0}, {"35", 603250, 0}, {"36", 609250, 0},
    {"37", 615250, 0}, {"38", 621250, 0}, {"39", 627250, 0},
    {"40", 633250, 0}, {"41", 639250, 0}, {"42", 645250, 0},
    {"43", 651250, 0}, {"44", 657250, 0}, {"45", 663250, 0},
    {"46", 669250, 0}, {"47", 675250, 0}, {"48", 681250, 0},
    {"49", 687250, 0}, {"50", 693250, 0}, {"51", 699250, 0},
    {"52", 705250, 0}, {"53", 711250, 0}, {"54", 717250, 0},
    {"55", 723250, 0}, {"56", 729250, 0}, {"57", 735250, 0},
    {"58", 741250, 0}, {"59", 747250, 0}, {"60", 753250, 0},
    {"61", 759250, 0}, {"62", 765250, 0},
    {0, 0}
};

/* JP cable */
static VideoFreqs ntsc_cable_jp[] = {
    {"13", 109250, 0}, {"14", 115250, 0}, {"15", 121250, 0},
    {"16", 127250, 0}, {"17", 133250, 0}, {"18", 139250, 0},
    {"19", 145250, 0}, {"20", 151250, 0}, {"21", 157250, 0},
    {"22", 165250, 0}, {"23", 223250, 0}, {"24", 231250, 0},
    {"25", 237250, 0}, {"26", 243250, 0}, {"27", 249250, 0},
    {"28", 253250, 0}, {"29", 259250, 0}, {"30", 265250, 0},
    {"31", 271250, 0}, {"32", 277250, 0}, {"33", 283250, 0},
    {"34", 289250, 0}, {"35", 295250, 0}, {"36", 301250, 0},
    {"37", 307250, 0}, {"38", 313250, 0}, {"39", 319250, 0},
    {"40", 325250, 0}, {"41", 331250, 0}, {"42", 337250, 0},
    {"43", 343250, 0}, {"44", 349250, 0}, {"45", 355250, 0},
    {"46", 361250, 0}, {"47", 367250, 0}, {"48", 373250, 0},
    {"49", 379250, 0}, {"50", 385250, 0}, {"51", 391250, 0},
    {"52", 397250, 0}, {"53", 403250, 0}, {"54", 409250, 0},
    {"55", 415250, 0}, {"56", 421250, 0}, {"57", 427250, 0},
    {"58", 433250, 0}, {"59", 439250, 0}, {"60", 445250, 0},
    {"61", 451250, 0}, {"62", 457250, 0}, {"63", 463250, 0},
    {0, 0}
};

/* australia */
static VideoFreqs pal_australia[] = {
    {"0", 46250, 0}, {"1", 57250, 0}, {"2", 64250, 0}, {"3", 86250, 0},
    {"4", 95250, 0}, {"5", 102250, 0}, {"5A", 138250, 0},
    {"6", 175250, 0}, {"7", 182250, 0}, {"8", 189250, 0},
    {"9", 196250, 0}, {"10", 209250, 0}, {"11", 216250, 0},
    {"28", 527250, 0}, {"29", 534250, 0}, {"30", 541250, 0},
    {"31", 548250, 0}, {"32", 555250, 0}, {"33", 562250, 0},
    {"34", 569250, 0}, {"35", 576250, 0}, {"36", 591250, 0},
    {"39", 604250, 0}, {"40", 611250, 0}, {"41", 618250, 0},
    {"42", 625250, 0}, {"43", 632250, 0}, {"44", 639250, 0},
    {"45", 646250, 0}, {"46", 653250, 0}, {"47", 660250, 0},
    {"48", 667250, 0}, {"49", 674250, 0}, {"50", 681250, 0},
    {"51", 688250, 0}, {"52", 695250, 0}, {"53", 702250, 0},
    {"54", 709250, 0}, {"55", 716250, 0}, {"56", 723250, 0},
    {"57", 730250, 0}, {"58", 737250, 0}, {"59", 744250, 0},
    {"60", 751250, 0}, {"61", 758250, 0}, {"62", 765250, 0},
    {"63", 772250, 0}, {"64", 779250, 0}, {"65", 786250, 0},
    {"66", 793250, 0}, {"67", 800250, 0}, {"68", 807250, 0},
    {"69", 814250, 0},
    {0, 0}
};

static VideoFreqs pal_australia_optus[] = {
    {"1", 138250, 0}, {"2", 147250, 0}, {"3", 154250, 0},
    {"4", 161250, 0}, {"5", 168250, 0}, {"6", 175250, 0},
    {"7", 182250, 0}, {"8", 189250, 0}, {"9", 196250, 0},
    {"10", 209250, 0}, {"11", 216250, 0}, {"12", 224250, 0},
    {"13", 231250, 0}, {"14", 238250, 0}, {"15", 245250, 0},
    {"16", 252250, 0}, {"17", 259250, 0}, {"18", 266250, 0},
    {"19", 273250, 0}, {"20", 280250, 0}, {"21", 287250, 0},
    {"22", 294250, 0}, {"23", 303250, 0}, {"24", 310250, 0},
    {"25", 317250, 0}, {"26", 324250, 0}, {"27", 338250, 0},
    {"28", 345250, 0}, {"29", 352250, 0}, {"30", 359250, 0},
    {"31", 366250, 0}, {"32", 373250, 0}, {"33", 380250, 0},
    {"34", 387250, 0}, {"35", 394250, 0}, {"36", 401250, 0},
    {"37", 408250, 0}, {"38", 415250, 0}, {"39", 422250, 0},
    {"40", 429250, 0}, {"41", 436250, 0}, {"42", 443250, 0},
    {"43", 450250, 0}, {"44", 457250, 0}, {"45", 464250, 0},
    {"46", 471250, 0}, {"47", 478250, 0}, {"48", 485250, 0},
    {0, 0}
};

/* europe                                                                */

/* CCIR frequencies */

#define FREQ_CCIR_I_III		\
    { "E2", 48250 ,0 },{ "E3", 55250 ,0 },{ "E4", 62250 ,0 },\
    { "S01", 69250 ,0 }, { "S02", 76250 ,0 },{ "S03", 83250 ,0 },\
    { "E5", 175250 ,0 },{ "E6", 182250 ,0 },{ "E7", 189250 ,0 },\
    { "E8", 196250 ,0 },{ "E9", 203250 ,0 },{ "E10", 210250 ,0 },\
    { "E11", 217250 ,0 },{ "E12", 224250 }

#define FREQ_CCIR_SL_SH		\
    { "SE1", 105250 ,0 },{ "SE2", 112250 ,0 },{ "SE3", 119250 ,0 },\
    { "SE4", 126250 ,0 },{ "SE5", 133250 ,0 },{ "SE6", 140250 ,0 },\
    { "SE7", 147250 ,0 },{ "SE8", 154250 ,0 },{ "SE9", 161250 ,0 },\
    { "SE10", 168250 ,0 },{ "SE11", 231250 ,0 },{ "SE12", 238250 ,0 },\
    { "SE13", 245250 ,0 },{ "SE14", 252250 ,0 },{ "SE15", 259250 ,0 },\
    { "SE16", 266250 ,0 },{ "SE17", 273250 ,0 },{ "SE18", 280250 ,0 },\
    { "SE19", 287250 ,0 },{ "SE20", 294250 }

#define FREQ_CCIR_H	\
    { "S21", 303250 ,0 },{ "S22", 311250 ,0 },{ "S23", 319250 ,0 },\
    { "S24", 327250 ,0 },{ "S25", 335250 ,0 },{ "S26", 343250 ,0 },\
    { "S27", 351250 ,0 }, { "S28", 359250 ,0 },{ "S29", 367250 ,0 },\
    { "S30", 375250 ,0 },{ "S31", 383250 ,0 },{ "S32", 391250 ,0 },\
    { "S33", 399250 ,0 },{ "S34", 407250 ,0 },{ "S35", 415250 ,0 },\
    { "S36", 423250 ,0 },{ "S37", 431250 ,0 },{ "S38", 439250 ,0 },\
    { "S39", 447250 ,0 },{ "S40", 455250 ,0 },{ "S41", 463250 }

/* OIRT frequencies */

#define FREQ_OIRT_I_III		\
    { "R1", 49750 ,0 },{ "R2", 59250 ,0 },{ "R3", 77250 ,0 },\
    { "R4", 85250 ,0 },{ "R5", 93250 ,0 },{ "R6", 175250 ,0 },\
    { "R7", 183250 ,0 },{ "R8", 191250 ,0 },{ "R9", 199250 ,0 },\
    { "R10", 207250 ,0 },{ "R11", 215250 ,0 },{ "R12", 223250 }

#define FREQ_OIRT_SL_SH		\
    { "SR1", 111250 ,0 },{ "SR2", 119250 ,0 },{ "SR3", 127250 ,0 },\
    { "SR4", 135250 ,0 },{ "SR5", 143250 ,0 },{ "SR6", 151250 ,0 },\
    { "SR7", 159250 ,0 },{ "SR8", 167250 ,0 },{ "SR11", 231250 ,0 },\
    { "SR12", 239250 ,0 },{ "SR13", 247250 ,0 },{ "SR14", 255250 ,0 },\
    { "SR15", 263250 ,0 },{ "SR16", 271250 ,0 },{ "SR17", 279250 ,0 },\
    { "SR18", 287250 ,0 },{ "SR19", 295250 }

#define FREQ_UHF	\
    { "21",471250 ,0 },{ "22",479250 ,0 },{ "23",487250 ,0 },\
    { "24",495250 ,0 },{ "25",503250 ,0 },{ "26",511250 ,0 },\
    { "27",519250 ,0 },{ "28",527250 ,0 },{ "29",535250 ,0 },\
    { "30",543250 ,0 },{ "31",551250 ,0 },{ "32",559250 ,0 },\
    { "33",567250 ,0 },{ "34",575250 ,0 },{ "35",583250 ,0 },\
    { "36",591250 ,0 },{ "37",599250 ,0 },{ "38",607250 ,0 },\
    { "39",615250 ,0 },{ "40",623250 ,0 },{ "41",631250 ,0 },\
    { "42",639250 ,0 },{ "43",647250 ,0 },{ "44",655250 ,0 },\
    { "45",663250 ,0 },{ "46",671250 ,0 },{ "47",679250 ,0 },\
    { "48",687250 ,0 },{ "49",695250 ,0 },{ "50",703250 ,0 },\
    { "51",711250 ,0 },{ "52",719250 ,0 },{ "53",727250 ,0 },\
    { "54",735250 ,0 },{ "55",743250 ,0 },{ "56",751250 ,0 },\
    { "57",759250 ,0 },{ "58",767250 ,0 },{ "59",775250 ,0 },\
    { "60",783250 ,0 },{ "61",791250 ,0 },{ "62",799250 ,0 },\
    { "63",807250 ,0 },{ "64",815250 ,0 },{ "65",823250 ,0 },\
    { "66",831250 ,0 },{ "67",839250 ,0 },{ "68",847250 ,0 },\
    { "69",855250 }

static VideoFreqs europe_west[] = {
    FREQ_CCIR_I_III,
    FREQ_CCIR_SL_SH,
    FREQ_CCIR_H,
    FREQ_UHF,
    {0, 0}
};

static VideoFreqs europe_east[] = {
    FREQ_OIRT_I_III,
    FREQ_OIRT_SL_SH,
    FREQ_CCIR_I_III,
    FREQ_CCIR_SL_SH,
    FREQ_CCIR_H,
    FREQ_UHF,
    {0, 0}
};

static VideoFreqs pal_italy[] = {
    {"A", 53750, 0}, {"B", 62250, 0}, {"C", 82250, 0},
    {"D", 175250, 0}, {"E", 183750, 0}, {"F", 192250, 0},
    {"G", 201250, 0}, {"H", 210250, 0}, {"H1", 217250, 0},
    {"H2", 224250, 0}, FREQ_UHF,
    {0, 0}
};

static VideoFreqs pal_ireland[] = {
    {"A0", 45750, 0}, {"A1", 48000, 0}, {"A2", 53750, 0},
    {"A3", 56000, 0}, {"A4", 61750, 0}, {"A5", 64000, 0},
    {"A6", 175250, 0}, {"A7", 176000, 0}, {"A8", 183250, 0},
    {"A9", 184000, 0}, {"A10", 191250, 0}, {"A11", 192000, 0},
    {"A12", 199250, 0}, {"A13", 200000, 0}, {"A14", 207250, 0},
    {"A15", 208000, 0}, {"A16", 215250, 0}, {"A17", 216000, 0},
    {"A18", 224000, 0}, {"A19", 232000, 0}, {"A20", 248000, 0},
    {"A21", 256000, 0}, {"A22", 264000, 0}, {"A23", 272000, 0},
    {"A24", 280000, 0}, {"A25", 288000, 0}, {"A26", 296000, 0},
    {"A27", 304000, 0}, {"A28", 312000, 0}, {"A29", 320000, 0},
    {"A30", 344000, 0}, {"A31", 352000, 0}, {"A32", 408000, 0},
    {"A33", 416000, 0}, {"A34", 448000, 0}, {"A35", 480000, 0},
    {"A36", 520000, 0}, FREQ_UHF,
    {0, 0}
};

static VideoFreqs secam_france[] = {
    {"K01", 47750, 0}, {"K02", 55750, 0}, {"K03", 60500, 0},
    {"K04", 63750, 0}, {"K05", 176000, 0}, {"K06", 184000, 0},
    {"K07", 192000, 0}, {"K08", 200000, 0}, {"K09", 208000, 0},
    {"K10", 216000, 0}, {"KB", 116750, 0}, {"KC", 128750, 0},
    {"KD", 140750, 0}, {"KE", 159750, 0}, {"KF", 164750, 0},
    {"KG", 176750, 0}, {"KH", 188750, 0}, {"KI", 200750, 0},
    {"KJ", 212750, 0}, {"KK", 224750, 0}, {"KL", 236750, 0},
    {"KM", 248750, 0}, {"KN", 260750, 0}, {"KO", 272750, 0},
    {"KP", 284750, 0}, {"KQ", 296750, 0}, {"H01", 303250, 0},
    {"H02", 311250, 0}, {"H03", 319250, 0}, {"H04", 327250, 0},
    {"H05", 335250, 0}, {"H06", 343250, 0}, {"H07", 351250, 0},
    {"H08", 359250, 0}, {"H09", 367250, 0}, {"H10", 375250, 0},
    {"H11", 383250, 0}, {"H12", 391250, 0}, {"H13", 399250, 0},
    {"H14", 407250, 0}, {"H15", 415250, 0}, {"H16", 423250, 0},
    {"H17", 431250, 0}, {"H18", 439250, 0}, {"H19", 447250, 0},
    FREQ_UHF,
    {0, 0}
};

static VideoFreqs pal_newzealand[] = {
    {"1", 45250, 0}, {"2", 55250, 0}, {"3", 62250, 0},
    {"4", 175250, 0}, {"5", 182250, 0}, {"6", 189250, 0},
    {"7", 196250, 0}, {"8", 203250, 0}, {"9", 210250, 0},
    {"10", 217250, 0}, {"11", 224250, 0},
    FREQ_UHF,
    {0, 0}
};

/* China broadcast */
static VideoFreqs pal_bcast_cn[] = {
    {"1", 49750, 0}, {"2", 57750, 0}, {"3", 65750, 0},
    {"4", 77250, 0}, {"5", 85250, 0}, {"6", 112250, 0},
    {"7", 120250, 0}, {"8", 128250, 0}, {"9", 136250, 0},
    {"10", 144250, 0}, {"11", 152250, 0}, {"12", 160250, 0},
    {"13", 168250, 0}, {"14", 176250, 0}, {"15", 184250, 0},
    {"16", 192250, 0}, {"17", 200250, 0}, {"18", 208250, 0},
    {"19", 216250, 0}, {"20", 224250, 0}, {"21", 232250, 0},
    {"22", 240250, 0}, {"23", 248250, 0}, {"24", 256250, 0},
    {"25", 264250, 0}, {"26", 272250, 0}, {"27", 280250, 0},
    {"28", 288250, 0}, {"29", 296250, 0}, {"30", 304250, 0},
    {"31", 312250, 0}, {"32", 320250, 0}, {"33", 328250, 0},
    {"34", 336250, 0}, {"35", 344250, 0}, {"36", 352250, 0},
    {"37", 360250, 0}, {"38", 368250, 0}, {"39", 376250, 0},
    {"40", 384250, 0}, {"41", 392250, 0}, {"42", 400250, 0},
    {"43", 408250, 0}, {"44", 416250, 0}, {"45", 424250, 0},
    {"46", 432250, 0}, {"47", 440250, 0}, {"48", 448250, 0},
    {"49", 456250, 0}, {"50", 463250, 0}, {"51", 471250, 0},
    {"52", 479250, 0}, {"53", 487250, 0}, {"54", 495250, 0},
    {"55", 503250, 0}, {"56", 511250, 0}, {"57", 519250, 0},
    {"58", 527250, 0}, {"59", 535250, 0}, {"60", 543250, 0},
    {"61", 551250, 0}, {"62", 559250, 0}, {"63", 607250, 0},
    {"64", 615250, 0}, {"65", 623250, 0}, {"66", 631250, 0},
    {"67", 639250, 0}, {"68", 647250, 0}, {"69", 655250, 0},
    {"70", 663250, 0}, {"71", 671250, 0}, {"72", 679250, 0},
    {"73", 687250, 0}, {"74", 695250, 0}, {"75", 703250, 0},
    {"76", 711250, 0}, {"77", 719250, 0}, {"78", 727250, 0},
    {"79", 735250, 0}, {"80", 743250, 0}, {"81", 751250, 0},
    {"82", 759250, 0}, {"83", 767250, 0}, {"84", 775250, 0},
    {"85", 783250, 0}, {"86", 791250, 0}, {"87", 799250, 0},
    {"88", 807250, 0}, {"89", 815250, 0}, {"90", 823250, 0},
    {"91", 831250, 0}, {"92", 839250, 0}, {"93", 847250, 0},
    {"94", 855250, 0}, {0, 0}
};

/* South Africa Broadcast */

static VideoFreqs pal_bcast_za[] = {
    {"1", 175250, 0}, {"2", 183250, 0}, {"3", 191250, 0},
    {"4", 199250, 0}, {"5", 207250, 0}, {"6", 215250, 0},
    {"7", 223250, 0}, {"8", 231250, 0}, FREQ_UHF,
    {0, 0}
};

static VideoFreqs argentina[] = {
    {"001", 56250, 0}, {"002", 62250, 0}, {"003", 68250, 0},
    {"004", 78250, 0}, {"005", 84250, 0}, {"006", 176250, 0},
    {"007", 182250, 0}, {"008", 188250, 0}, {"009", 194250, 0},
    {"010", 200250, 0}, {"011", 206250, 0}, {"012", 212250, 0},
    {"013", 122250, 0}, {"014", 128250, 0}, {"015", 134250, 0},
    {"016", 140250, 0}, {"017", 146250, 0}, {"018", 152250, 0},
    {"019", 158250, 0}, {"020", 164250, 0}, {"021", 170250, 0},
    {"022", 218250, 0}, {"023", 224250, 0}, {"024", 230250, 0},
    {"025", 236250, 0}, {"026", 242250, 0}, {"027", 248250, 0},
    {"028", 254250, 0}, {"029", 260250, 0}, {"030", 266250, 0},
    {"031", 272250, 0}, {"032", 278250, 0}, {"033", 284250, 0},
    {"034", 290250, 0}, {"035", 296250, 0}, {"036", 302250, 0},
    {"037", 308250, 0}, {"038", 314250, 0}, {"039", 320250, 0},
    {"040", 326250, 0}, {"041", 332250, 0}, {"042", 338250, 0},
    {"043", 344250, 0}, {"044", 350250, 0}, {"045", 356250, 0},
    {"046", 362250, 0}, {"047", 368250, 0}, {"048", 374250, 0},
    {"049", 380250, 0}, {"050", 386250, 0}, {"051", 392250, 0},
    {"052", 398250, 0}, {"053", 404250, 0}, {"054", 410250, 0},
    {"055", 416250, 0}, {"056", 422250, 0}, {"057", 428250, 0},
    {"058", 434250, 0}, {"059", 440250, 0}, {"060", 446250, 0},
    {"061", 452250, 0}, {"062", 458250, 0}, {"063", 464250, 0},
    {"064", 470250, 0}, {"065", 476250, 0}, {"066", 482250, 0},
    {"067", 488250, 0}, {"068", 494250, 0}, {"069", 500250, 0},
    {"070", 506250, 0}, {"071", 512250, 0}, {"072", 518250, 0},
    {"073", 524250, 0}, {"074", 530250, 0}, {"075", 536250, 0},
    {"076", 542250, 0}, {"077", 548250, 0}, {"078", 554250, 0},
    {"079", 560250, 0}, {"080", 566250, 0}, {"081", 572250, 0},
    {"082", 578250, 0}, {"083", 584250, 0}, {"084", 590250, 0},
    {"085", 596250, 0}, {"086", 602250, 0}, {"087", 608250, 0},
    {"088", 614250, 0}, {"089", 620250, 0}, {"090", 626250, 0},
    {"091", 632250, 0}, {"092", 638250, 0}, {"093", 644250, 0},
    {0, 0}
};

static VideoChannels channels[] = {
    {"us-cable", ntsc_cable},
    {"us-bcast", ntsc_bcast},
    {"us-cable-hrc", ntsc_hrc},
    {"japan-bcast", ntsc_bcast_jp},
    {"japan-cable", ntsc_cable_jp},
    {"europe-west", europe_west},
    {"europe-east", europe_east},
    {"italy", pal_italy},
    {"newzealand", pal_newzealand},
    {"australia", pal_australia},
    {"ireland", pal_ireland},
    {"france", secam_france},
    {"china-bcast", pal_bcast_cn},
    {"southafrica", pal_bcast_za},
    {"argentina", argentina},
    {"canada-cable", ntsc_cable_ca},
    {"australia-optus", pal_australia_optus},
    {0, 0}
};

static VideoNorms norms[] = {
    {"NTSC", {1, V4L2_STD_NTSC_M}, 720, 480},
    {"NTSC-M", {1, V4L2_STD_NTSC_M}, 720, 480},
    {"NTSC-M-JP", {6, V4L2_STD_NTSC_M_JP}, 720, 480},
    {"PAL", {0, V4L2_STD_PAL_B}, 768, 576},
    {"PAL-B", {0, V4L2_STD_PAL_B}, 768, 576},
    {"PAL-B1", {0, V4L2_STD_PAL_B1}, 768, 576},
    {"PAL-G", {0, V4L2_STD_PAL_G}, 768, 576},
    {"PAL-H", {0, V4L2_STD_PAL_H}, 768, 576},
    {"PAL-I", {0, V4L2_STD_PAL_I}, 768, 576},
    {"PAL-D", {0, V4L2_STD_PAL_D}, 768, 576},
    {"PAL-D1", {0, V4L2_STD_PAL_D1}, 768, 576},
    {"PAL-K", {0, V4L2_STD_PAL_K}, 768, 576},
    {"PAL-M", {4, V4L2_STD_PAL_M}, 768, 576},
    {"PAL-N", {5, V4L2_STD_PAL_N}, 768, 576},
    {"PAL-Nc", {3, V4L2_STD_PAL_Nc}, 768, 576},
    {"PAL-60", {7, V4L2_STD_PAL_60}, 768, 576},
    {"SECAM-B", {2, V4L2_STD_SECAM_B}, 768, 576},
    {"SECAM-D", {2, V4L2_STD_SECAM_D}, 768, 576},
    {"SECAM-G", {2, V4L2_STD_SECAM_G}, 768, 576},
    {"SECAM-H", {2, V4L2_STD_SECAM_H}, 768, 576},
    {"SECAM-K", {2, V4L2_STD_SECAM_K}, 768, 576},
    {"SECAM-K1", {2, V4L2_STD_SECAM_K1}, 768, 576},
    {"SECAM-L", {2, V4L2_STD_SECAM_L}, 768, 576},
    {"ATSC-8-VSB", {1, V4L2_STD_ATSC_8_VSB}, 720, 480},
    {"ATSC-16-VSB", {1, V4L2_STD_ATSC_16_VSB}, 720, 480},
    {0, -1, 0}
};

TVObject::TVObject(const char *aname):DynamicObject(aname)
{
    type = TYPE_TV;
    fd = -1;
    version = 2;
    audio = -1;
    input = 0;
    norm = 0;
    depth = 4;
    muted = 0;
    chanlist = 0;
    channel = 0;
    device = "/dev/video0";

    MemberFunctionProperty < TVObject > *mp;
    mp = new MemberFunctionProperty < TVObject > ("info", this, &TVObject::pget_Info, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("mute", this, &TVObject::pget_Mute, &TVObject::pset_Mute, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("volume", this, &TVObject::pget_Volume, &TVObject::pset_Volume, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("hue", this, &TVObject::pget_Hue, &TVObject::pset_Hue, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("contrast", this, &TVObject::pget_Contrast, &TVObject::pset_Contrast, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("saturation", this, &TVObject::pget_Saturation, &TVObject::pset_Saturation, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("brightness", this, &TVObject::pget_Brightness, &TVObject::pset_Brightness, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("norm", this, &TVObject::pget_Norm, &TVObject::pset_Norm, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("version", this, &TVObject::pget_Version, &TVObject::pset_Version, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("input", this, &TVObject::pget_Input, &TVObject::pset_Input, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("audio", this, &TVObject::pget_Audio, &TVObject::pset_Audio, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("device", this, &TVObject::pget_Device, &TVObject::pset_Device, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("chanlist", this, &TVObject::pget_ChanList, &TVObject::pset_ChanList, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("channel", this, &TVObject::pget_Channel, &TVObject::pset_Channel, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("depth", this, &TVObject::pget_Depth, &TVObject::pset_Depth, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("frequency", this, &TVObject::pget_Frequency, &TVObject::pset_Frequency, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("status", this, &TVObject::pget_Status, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("cardtype", this, &TVObject::pget_CardType, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("videoinputs", this, &TVObject::pget_VideoInputs, NULL, false);
    AddProperty(mp);
    mp = new MemberFunctionProperty < TVObject > ("audioinputs", this, &TVObject::pget_AudioInputs, NULL, false);
    AddProperty(mp);

    MemberMethodHandler < TVObject > *mh;
    mh = new MemberMethodHandler < TVObject > ("open", this, 0, &TVObject::m_Open);
    AddMethod(mh);
    mh = new MemberMethodHandler < TVObject > ("close", this, 0, &TVObject::m_Close);
    AddMethod(mh);
    mh = new MemberMethodHandler < TVObject > ("prev", this, 0, &TVObject::m_Prev);
    AddMethod(mh);
    mh = new MemberMethodHandler < TVObject > ("next", this, 0, &TVObject::m_Next);
    AddMethod(mh);
    mh = new MemberMethodHandler < TVObject > ("getfrequency", this, 1, &TVObject::m_GetFrequency);
    AddMethod(mh);
    mh = new MemberMethodHandler < TVObject > ("getchannel", this, 1, &TVObject::m_GetChannel);
    AddMethod(mh);
    mh = new MemberMethodHandler < TVObject > ("mixer", this, 3, &TVObject::m_Mixer);
    AddMethod(mh);
}

TVObject::~TVObject(void)
{
    TVClose();
}

int TVObject::TVOpen(void)
{
    if (fd > 0) {
        return fd;
    }
    fd = open(device.c_str(), O_RDWR);
    if (fd == -1) {
        systemObject->Log(0, "TVOpen: open: %s: %s", device.c_str(), strerror(errno));
        return fd;
    }
    // Check driver V4L2 capabilities
    if (version == 2) {
        memset(&v2caps, 0, sizeof(v2caps));
        if (ioctl(fd, VIDIOC_QUERYCAP, &v2caps) < 0) {
            version = 1;
            systemObject->Log(0, "RadioOpen: VIDIOC_QUERYCAP: %s: switch to v4l1", strerror(errno));
        }
    }
    switch (version) {
    case 1:
        if (ioctl(fd, VIDIOCGPICT, &vpic) == -1) {
            systemObject->Log(0, "TVOpen: VIDIOCGPICT: %s", strerror(errno));
            return TVClose();
        }
        // Get device capabilities
        if (ioctl(fd, VIDIOCGCAP, &vcaps) == -1) {
            systemObject->Log(0, "TVOpen: VIDIOCGCAP: %s", strerror(errno));
            return TVClose();
        }
        if (ioctl(fd, VIDIOCGTUNER, &vtuner) == -1) {
            systemObject->Log(0, "TVOpen: VIDIOCGTUNER: %s", strerror(errno));
            return TVClose();
        }
        flags = vtuner.flags;
        break;

    case 2:
        // Current picture format
        memset(&v2fmt, 0, sizeof(v2fmt));
        v2fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(fd, VIDIOC_G_FMT, &v2fmt) < 0) {
            systemObject->Log(0, "TVOpen: VIDIOC_G_FMT: %s", strerror(errno));
            return TVClose();
        }
        // Get device capabilities
        memset(&v2tuner, 0, sizeof(v2tuner));
        v2tuner.index = input;
        if (ioctl(fd, VIDIOC_G_TUNER, &v2tuner) < 0) {
            systemObject->Log(0, "TVOpen: VIDIOC_G_TUNER: %d: %s", input, strerror(errno));
        }
        flags = v2tuner.capability;
        break;
    }
    // Figure out/adjust resolution
    switch (version) {
    case 1:
        if (area.w > vcaps.maxwidth) {
            area.w = vcaps.maxwidth;
        }
        if (area.h > vcaps.maxheight) {
            area.w = vcaps.maxheight;
        }
        break;
    case 2:
        break;
    }
    if (area.w <= 0 || area.w > norms[norm].width) {
        area.w = norms[norm].width;
    }
    if (area.h <= 0 || area.h > norms[norm].height) {
        area.h = norms[norm].height;
    }
    return fd;
}

int TVObject::TVInput(void)
{
    if (fd < 0) {
        return fd;
    }
    switch (version) {
    case 1:
        vtuner.tuner = input;
        if (ioctl(fd, VIDIOCGTUNER, &vtuner) < 0) {
            systemObject->Log(0, "TVInput: VIDIOCGTUNER: %d: %s", input, strerror(errno));
            return -1;
        }
        vchan.channel = input;
        if (ioctl(fd, VIDIOCGCHAN, &vchan) < 0) {
            systemObject->Log(0, "TVInput: VIDIOCGCHAN: %d: %s", input, strerror(errno));
            return -1;
        }
        vchan.channel = input;
        vchan.norm = norms[norm].id[version - 1];
        if (ioctl(fd, VIDIOCSCHAN, &vchan) < 0) {
            systemObject->Log(0, "TVInput: VIDIOCSCHAN: %d/%s: %s", norm, norms[norm].id[version - 1], strerror(errno));
            return -1;
        }
        break;

    case 2:
        if (ioctl(fd, VIDIOC_S_STD, &norms[norm].id[version - 1]) < 0) {
            systemObject->Log(0, "TVInput: VIDIOC_S_STD: %d/%s: %s", norm, norms[norm].id[version - 1], strerror(errno));
            return -1;
        }
        if (ioctl(fd, VIDIOC_S_INPUT, &input) < 0) {
            systemObject->Log(0, "TVInput: VIDIOC_S_INPUT: %d: %s", input, strerror(errno));
            return -1;
        }
        // Re-read tuner capabilities
        if (input != v2tuner.index) {
            memset(&v2tuner, 0, sizeof(v2tuner));
            v2tuner.index = input;
            // For some reason PVR150 fails on first call
            if (ioctl(fd, VIDIOC_G_TUNER, &v2tuner) < 0 &&
                ioctl(fd, VIDIOC_G_TUNER, &v2tuner) < 0) {
                systemObject->Log(0, "TVInput: VIDIOC_G_TUNER: %d: %s", input, strerror(errno));
            }
            flags = v2tuner.capability;
        }
        if (audio > -1 && ioctl(fd, VIDIOC_S_AUDIO, &audio) < 0) {
            systemObject->Log(0, "TVInput: VIDIOC_S_AUIDO: %d: %s", audio, strerror(errno));
            return -1;
        }
        break;
    }
    systemObject->Log(0, "TVInput: %d: Norm=%d, Video=%d, Audio=%d", version, norm, input, audio);
    return 0;
}

int TVObject::TVSetChannel(void)
{
    if (fd < 0) {
        return fd;
    }
    int freq = channels[chanlist].freqs[channel].freq2 ? 
                   channels[chanlist].freqs[channel].freq2 :
                       channels[chanlist].freqs[channel].freq;

    switch (version) {
    case 1:
        freq = freq * 16;
        if (!(flags & VIDEO_TUNER_LOW)) {
            freq /= 1000;
        }
        if (ioctl(fd, VIDIOCSFREQ, &freq) < 0) {
            systemObject->Log(0, "TVSetChannel: VIDIOCSFREQ: %d: %s", freq, strerror(errno));
            return -1;
        }
        break;

    case 2:
        freq = freq * 16;
        if (!(flags & V4L2_TUNER_CAP_LOW)) {
            freq /= 1000;
        }
        memset(&v2freq, 0, sizeof(v2freq));
        v2freq.frequency = freq;
        v2freq.type = V4L2_TUNER_ANALOG_TV;
        if (ioctl(fd, VIDIOC_S_FREQUENCY, &v2freq) < 0) {
            systemObject->Log(0, "TVSetChannel: VIDIOC_S_FREQUENCY: %d: %d: %s", input, freq, strerror(errno));
            return -1;
        }
        break;
    }
    systemObject->Log(0, "v4l: %d: TV Channel = %s/%s/%d", version, norms[norm].name, channels[chanlist].name, channels[chanlist].freqs[channel].name, freq);
    return 0;
}

const char *TVObject::TVGetChannel(void)
{
    return channels[chanlist].freqs[channel].name;
}

const char *TVObject::TVGetNorm(void)
{
    return norms[norm].name;
}

int TVObject::TVClose(void)
{
    if (fd < 0) {
        return fd;
    }
    close(fd);
    fd = -1;
    return fd;
}

int TVObject::TVGetMute(void)
{
    return muted;
}

int TVObject::TVSetMute(int flag)
{
    muted = flag > 0 ? 1 : 0;
    TVSetMixer("Unmute:Line", muted ? 0 : 1);
    if (fd < 0) {
        return fd;
    }
    switch (version) {
    case 1:
        if (ioctl(fd, VIDIOCGAUDIO, &vaudio) < 0) {
            systemObject->Log(0, "TVMute: VIDIOCGAUDIO: %d: %s", muted, strerror(errno));
            return -1;
        }
        if (muted) {
            vaudio.volume = 0;
            vaudio.flags |= VIDEO_AUDIO_MUTE;
        } else {
            vaudio.volume = 65535;
            vaudio.mode = VIDEO_SOUND_STEREO;
            vaudio.flags &= ~VIDEO_AUDIO_MUTE;
        }
        if (ioctl(fd, VIDIOCSAUDIO, &vaudio) < 0) {
            systemObject->Log(0, "TVMute: VIDIOCSAUDIO: %s", strerror(errno));
            return -1;
        }
        break;

    case 2:
        v2ctrl.id = V4L2_CID_AUDIO_MUTE;
        v2ctrl.value = muted;
        if (ioctl(fd, VIDIOC_S_CTRL, &v2ctrl) == -1) {
            systemObject->Log(0, "TVSetMute: VIDIOC_S_CTRL: %d: %s", muted, strerror(errno));
        }
        break;
    }
    return 0;
}

int TVObject::TVGetPicture(int what)
{
    if (fd < 0) {
        return fd;
    }
    int val = 0;
    switch (version) {
    case 1:
        switch (what) {
        case 0:
            val = vpic.hue;
            break;
        case 1:
            val = vpic.colour;
            break;
        case 2:
            val = vpic.contrast;
            break;
        case 3:
            val = vpic.brightness;
            break;
        }


    case 2:
        switch (what) {
        case 0:
            v2ctrl.id = V4L2_CID_HUE;
            break;
        case 1:
            v2ctrl.id = V4L2_CID_SATURATION;
            break;
        case 2:
            v2ctrl.id = V4L2_CID_CONTRAST;
            break;
        case 3:
            v2ctrl.id = V4L2_CID_BRIGHTNESS;
            break;
        default:
            return -1;
        }
        if (ioctl(fd, VIDIOC_G_CTRL, &v2ctrl) < 0) {
            systemObject->Log(0, "TVGetPicture: VIDIOC_G_CTRL: %d: %s", what, strerror(errno));
            return -1;
        }
        val = v2ctrl.value;
    }
    return (int) (((val / 65535.0) * 100.0) + 0.5);
}

int TVObject::TVSetPicture(int what, int value)
{
    if (fd < 0) {
        return fd;
    }
    int val = value;
    if (val > 100) {
        val = 100;
    }
    if (val < 0) {
        val = 0;
    }
    val = (int) (((val / 100.0) * 65535.0) + 0.5);
    switch (version) {
    case 1:
        switch (what) {
        case 0:
            vpic.hue = val;
            break;
        case 1:
            vpic.colour = val;
            break;
        case 2:
            vpic.contrast = val;
            break;
        case 3:
            vpic.brightness = val;
            break;
        default:
            return -1;
        }
        Lock();
        if (ioctl(fd, VIDIOCSPICT, &vpic) == -1) {
            systemObject->Log(0, "TVSetPicture: VIDIOCSPICT: %d: %d: %s", what, val, strerror(errno));
        }
        Unlock();
        break;

    case 2:
        switch (what) {
        case 0:
            v2ctrl.id = V4L2_CID_HUE;
            break;
        case 1:
            v2ctrl.id = V4L2_CID_SATURATION;
            break;
        case 2:
            v2ctrl.id = V4L2_CID_CONTRAST;
            break;
        case 3:
            v2ctrl.id = V4L2_CID_BRIGHTNESS;
            break;
        default:
            return -1;
        }
        v2ctrl.value = val;
        Lock();
        if (ioctl(fd, VIDIOC_S_CTRL, &v2ctrl) == -1) {
            systemObject->Log(0, "TVSetPicture: VIDIOC_S_CTRL: %d: %d: %s", what, val, strerror(errno));
        }
        Unlock();
        break;
    }
    return 0;
}

int TVObject::TVSetMixer(char *name, int value)
{
#ifdef HAVE_ALSA
    snd_mixer_t *handle;
    long vmin, vmax, vol;
    snd_mixer_elem_t *elem;
    int chn, err, mode = 0;
    snd_mixer_selem_id_t *sid;

    if ((err = snd_mixer_open(&handle, 0)) < 0) {
        systemObject->Log(0, "TVSetMixer: open error: %s", snd_strerror(err));
        return err;
    }
    if ((err = snd_mixer_attach(handle, "default")) < 0) {
        systemObject->Log(0, "TVSetMixer: attach error: %s", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }
    if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        systemObject->Log(0, "TVSetMixer: register error: %s", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }
    if ((err = snd_mixer_load(handle)) < 0) {
        systemObject->Log(0, "TVSetMixer: load error: %s", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }
    if (!strncasecmp(name, "Unmute:", 7)) {
        name += 7;
        mode = 1;
    }
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, name);
    if (!(elem = snd_mixer_find_selem(handle, sid))) {
        systemObject->Log(0, "TVSetMixer: unable to find simple control %s", name);
        snd_mixer_close(handle);
        return -1;
    }
    snd_mixer_selem_get_playback_volume_range(elem, &vmin, &vmax);
    for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
        if (!snd_mixer_selem_has_playback_channel(elem, (snd_mixer_selem_channel_id_t) chn)) {
            continue;
        }
        switch (mode) {
        case 1:
            snd_mixer_selem_set_playback_switch(elem, (snd_mixer_selem_channel_id_t)chn, value);
            break;
        default:
            snd_mixer_selem_set_playback_volume(elem, (snd_mixer_selem_channel_id_t)
                                                chn, value < vmin ? vmin : value > vmax ? vmax : value);
        }
    }
    snd_mixer_close(handle);
    return 0;
#else
    return -1;
#endif
}

long TVObject::TVGetMixer(char *name)
{
#ifdef HAVE_ALSA
    snd_mixer_t *handle;
    long vmin, vmax, vol;
    int chn, err, mode = 0;
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;

    if ((err = snd_mixer_open(&handle, 0)) < 0) {
        systemObject->Log(0, "TVGetMixer: open error: %s", snd_strerror(err));
        return err;
    }
    if ((err = snd_mixer_attach(handle, "default")) < 0) {
        systemObject->Log(0, "TVGetMixer: attach error: %s", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }
    if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        systemObject->Log(0, "TVGetMixer: register error: %s", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }
    if ((err = snd_mixer_load(handle)) < 0) {
        systemObject->Log(0, "TVGetMixer: load error: %s", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }
    if (!strncasecmp(name, "Unmute:", 7)) {
        name += 7;
        mode = 1;
    } else
    if (!strncmp(name, "Max:", 4)) {
        name += 4;
        mode = 2;
    } else
    if (!strncmp(name, "Min:", 4)) {
        name += 4;
        mode = 3;
    }
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, name);
    if (!(elem = snd_mixer_find_selem(handle, sid))) {
        systemObject->Log(0, "TVGetMixer: unable to find simple control %s", name);
        snd_mixer_close(handle);
        snd_mixer_selem_id_free(sid);
        return -1;
    }
    snd_mixer_selem_get_playback_volume_range(elem, &vmin, &vmax);
    if (mode == 2) {
        vol = vmax;
    } else
    if (mode == 3) {
        vol = vmin;
    }
    for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
        if (!snd_mixer_selem_has_playback_channel(elem, (snd_mixer_selem_channel_id_t) chn)) {
            continue;
        }
        switch (mode) {
        case 1:
            snd_mixer_selem_get_playback_switch(elem, (snd_mixer_selem_channel_id_t) chn, (int *) &vol);
            break;
        case 0:
            snd_mixer_selem_get_playback_volume(elem, (snd_mixer_selem_channel_id_t) chn, &vol);
            break;
        }
        break;
    }
    snd_mixer_close(handle);
    return vol;
#else
    return -1;
#endif
}

Variant TVObject::pget_Status(void)
{
    return anytovariant("idle");
}

Variant TVObject::pget_Version(void)
{
    return anytovariant(version);
}

int TVObject::pset_Version(Variant value)
{
    int v = value;
    switch (v) {
    case 1:
    case 2:
        version = value;
        break;
    }
    TVClose();
    TVOpen();
    return 0;
}


Variant TVObject::pget_Depth(void)
{
    return anytovariant(depth);
}

int TVObject::pset_Depth(Variant value)
{
    depth = value;
    return 0;
}

Variant TVObject::pget_Volume(void)
{
    int rc = 0;
    return anytovariant(rc);
}

int TVObject::pset_Volume(Variant value)
{
    return 0;
}

Variant TVObject::pget_Mute(void)
{
    return anytovariant(TVGetMute());
}

int TVObject::pset_Mute(Variant value)
{
    TVSetMute(value);
    return 0;
}

Variant TVObject::pget_Hue(void)
{
    return anytovariant(TVGetPicture(0));
}

int TVObject::pset_Hue(Variant value)
{
    TVSetPicture(0, value);
    return 0;
}

Variant TVObject::pget_Saturation(void)
{
    return anytovariant(TVGetPicture(1));
}

int TVObject::pset_Saturation(Variant value)
{
    TVSetPicture(1, value);
    return 0;
}

Variant TVObject::pget_Brightness(void)
{
    return anytovariant(TVGetPicture(3));
}

int TVObject::pset_Brightness(Variant value)
{
    TVSetPicture(3, value);
    return 0;
}

Variant TVObject::pget_Contrast(void)
{
    return anytovariant(TVGetPicture(2));
}

int TVObject::pset_Contrast(Variant value)
{
    TVSetPicture(2, value);
    return 0;
}

Variant TVObject::pget_ChanList(void)
{
    return anytovariant(channels[chanlist].name);
}

int TVObject::pset_ChanList(Variant value)
{
    const char *name = value;
    for (int i = 0; channels[i].name; i++) {
        if (!strcasecmp(channels[i].name, name)) {
            chanlist = i;
            channel = 0;
            break;
        }
    }
    return 0;
}

Variant TVObject::pget_Norm(void)
{
    return anytovariant(TVGetNorm());
}

int TVObject::pset_Norm(Variant value)
{
    const char *name = value;

    for (int i = 0; norms[i].id >= 0; i++) {
        if (!strcasecmp(norms[i].name, name)) {
            norm = i;
            break;
        }
    }
    pset_Input(anytovariant(input));
    return 0;
}

Variant TVObject::pget_Device(void)
{
    return anytovariant(device.c_str());
}

int TVObject::pset_Device(Variant value)
{
    device = (const char *) value;
    TVClose();
    TVOpen();
    return 0;
}

Variant TVObject::pget_Input(void)
{
    return anytovariant(input);
}

int TVObject::pset_Input(Variant value)
{
    input = value;
    TVInput();
    return 0;
}

Variant TVObject::pget_Audio(void)
{
    return anytovariant(audio);
}

int TVObject::pset_Audio(Variant value)
{
    audio = value;
    TVInput();
    return 0;
}

Variant TVObject::pget_Channel(void)
{
    return anytovariant(TVGetChannel());
}

int TVObject::pset_Channel(Variant value)
{
    const char *name = value;
    for (int i = 0; channels[chanlist].freqs[i].name; i++) {
        if (!strcmp(channels[chanlist].freqs[i].name, name)) {
            channel = i;
            break;
        }
    }
    TVSetChannel();
    return 0;
}

Variant TVObject::pget_Frequency(void)
{
    int freq = channels[chanlist].freqs[channel].freq2 ? 
                   channels[chanlist].freqs[channel].freq2 : 
                       channels[chanlist].freqs[channel].freq;
    return anytovariant(freq);
}

int TVObject::pset_Frequency(Variant value)
{
    channels[chanlist].freqs[channel].freq2 = value;
    if (channels[chanlist].freqs[channel].freq2 == channels[chanlist].freqs[channel].freq) {
        channels[chanlist].freqs[channel].freq2 = 0;
    }
    TVSetChannel();
    return 0;
}

Variant TVObject::pget_VideoInputs(void)
{
    if (TVOpen() < 0) {
        return VARNULL;
    }
    string info;
    switch (version) {
    case 1:
        for (int i = 0; i < vcaps.channels; i++) {
             vchan.channel = i;
             if (ioctl(fd, VIDIOCGCHAN, &vchan) == -1) {
                 systemObject->Log(0, "pget_VideoInputs: VIDIOCGCHAN: %s", strerror(errno));
                 return VARNULL;
             }
             info += intString(i);
             info += " {";
             info += vchan.name;
             info += "} ";
             info += (vchan.type & VIDEO_TYPE_TV ? "tv" : "camera");
             info += " ";
        }
        break;

    case 2:
        v2input.index = 0;
        while (ioctl(fd, VIDIOC_ENUMINPUT, &v2input) >= 0) {
            info += intString(v2input.index);
            info += " {";
            info += (char*)(v2input.name);
            info += "} ";
            info += (v2input.type == V4L2_INPUT_TYPE_TUNER ? "tv" : "camera");
            info += " ";
            v2input.index++;
        }
        break;
    }
    return anytovariant(info.c_str());
}

Variant TVObject::pget_AudioInputs(void)
{
    if (TVOpen() < 0) {
        return VARNULL;
    }
    string info;
    switch (version) {
    case 1:
        for (int i = 0; i < vcaps.audios; i++) {
            vaudio.audio = i;
            if (ioctl(fd, VIDIOCGAUDIO, &vaudio) == -1) {
                systemObject->Log(0, "pget_AudioInputs: VIDIOCGAUDIO: %s", strerror(errno));
                return VARNULL;
            }
            info += intString(i);
            info += " {";
            info += vaudio.name;
            info += "} ";
        }
        break;

    case 2:
        v2audio.index = 0;
        while (ioctl(fd, VIDIOC_ENUMAUDIO, &v2audio) >= 0) {
            info += intString(v2audio.index);
            info += " {";
            info += (char*)v2audio.name;
            info += "} ";
            v2audio.index++;
        }
        break;
    }
    return anytovariant(info.c_str());
}

Variant TVObject::pget_CardType(void)
{
    if (TVOpen() < 0) {
        return VARNULL;
    }
    string info;
    switch (version) {
    case 1:
        info += vcaps.name;
        break;

    case 2:
        info += (char*)v2caps.card;
        break;
    }
    return anytovariant(info.c_str());
}

Variant TVObject::pget_Info(void)
{
    if (TVOpen() < 0) {
        return VARNULL;
    }
    string info;
    char buffer[255];
    int max_tuner = 0;
    char *yn[2] = { "YES", "NO" };

    switch (version) {
    case 1:
        sprintf(buffer, "Device Info: %s\n", vcaps.name);
        info += buffer;
        sprintf(buffer, "Can capture: %s\n", yn[!(vcaps.type & VID_TYPE_CAPTURE)]);
        info += buffer;
        sprintf(buffer, "Can clip: %s\n", yn[!(vcaps.type & VID_TYPE_CLIPPING)]);
        info += buffer;
        sprintf(buffer, "Channels: %i\n", vcaps.channels);
        info += buffer;
        sprintf(buffer, "Has tuner: %s\n", yn[!(vcaps.type & VID_TYPE_TUNER)]);
        info += buffer;
        sprintf(buffer, "Ovl overwrites: %s\n", yn[!(vcaps.type & VID_TYPE_FRAMERAM)]);
        info += buffer;
        sprintf(buffer, "Audio devices: %i\n", vcaps.audios);
        info += buffer;
        sprintf(buffer, "Has teletext: %s\n", yn[!(vcaps.type & VID_TYPE_TELETEXT)]);
        info += buffer;
        sprintf(buffer, "Can scale: %s\n", yn[!(vcaps.type & VID_TYPE_SCALES)]);
        info += buffer;
        sprintf(buffer, "Width min-max: %i-%i\n", vcaps.minwidth, vcaps.maxwidth);
        info += buffer;
        sprintf(buffer, "Height min-max: %i-%i\n", vcaps.minheight, vcaps.maxheight);
        info += buffer;
        sprintf(buffer, "Can overlay: %s\n", yn[!(vcaps.type & VID_TYPE_OVERLAY)]);
        info += buffer;
        sprintf(buffer, "Monochrome: %s\n", yn[!(vcaps.type & VID_TYPE_MONOCHROME)]);
        info += buffer;
        sprintf(buffer, "Can chromakey: %s\n", yn[!(vcaps.type & VID_TYPE_CHROMAKEY)]);
        info += buffer;
        sprintf(buffer, " Can subcapture: %s\n", yn[!(vcaps.type & VID_TYPE_SUBCAPTURE)]);
        info += buffer;
        for (int i = 0; i < vcaps.channels; i++) {
            vchan.channel = i;
            if (ioctl(fd, VIDIOCGCHAN, &vchan) == -1) {
                systemObject->Log(0, "pget_Info: VIDIOCGCHAN: %s", strerror(errno));
                return VARNULL;
            }
            sprintf(buffer, "Channel %i: %s (%s)\n", i, vchan.name, vchan.type & VIDEO_TYPE_TV ? "tv" : "camera");
            info += buffer;
            sprintf(buffer, "Tuners: %i\n", vchan.tuners);
            info += buffer;
            sprintf(buffer, "Has audio: %s\n ", yn[!(vchan.flags & VIDEO_VC_AUDIO)]);
            info += buffer;
            if (vchan.tuners > max_tuner)
                max_tuner = vchan.tuners;
        }
        for (int i = 0; i < vcaps.audios; i++) {
            vaudio.audio = i;
            if (ioctl(fd, VIDIOCGAUDIO, &vaudio) == -1) {
                systemObject->Log(0, "pget_Info: VIDIOCGAUDIO: %s", strerror(errno));
                return VARNULL;
            }
            sprintf(buffer, "Audio %i: %s\n", i, vaudio.name);
            info += buffer;
            sprintf(buffer, "Controllable: %s%s%s%s\n",
                    (vaudio.flags & VIDEO_AUDIO_MUTABLE ? "Muting " : ""),
                    (vaudio.flags & VIDEO_AUDIO_VOLUME ? "Volume " : ""),
                    (vaudio.flags & VIDEO_AUDIO_BASS ? "Bass " : ""),
                    (vaudio.flags & VIDEO_AUDIO_TREBLE ? "Treble " : ""));
            info += buffer;
        }
        struct video_mbuf vmbuf;
        /* Retrieve sizes and offsets */
        if (ioctl(fd, VIDIOCGMBUF, &vmbuf) == -1) {
            systemObject->Log(0, "pget_Info: VIDIOCGMBUF: %s", strerror(errno));
            return VARNULL;
        }
        /* Print memory info */
        sprintf(buffer, "Memory Map of %i frames: %i bytes\n", vmbuf.frames, vmbuf.size);
        info += buffer;
        for (int i = 0; i < vmbuf.frames; i++) {
            sprintf(buffer, "Offset of frame %i: %i\n", i, vmbuf.offsets[i]);
            info += buffer;
        }
        break;

    case 2:
        sprintf(buffer, "Driver name   : %s\n", v2caps.driver);
        info += buffer;
        sprintf(buffer, "Card type     : %s\n", v2caps.card);
        info += buffer;
        sprintf(buffer, "Bus info      : %s\n", v2caps.bus_info);
        info += buffer;
        sprintf(buffer, "Driver version: %d\n", v2caps.version);
        info += buffer;
        sprintf(buffer, "Capabilities  : 0x%08X\n", v2caps.capabilities);
        info += buffer;

        v2input.index = 0;
        info += "Video inputs:\n";
        while (ioctl(fd, VIDIOC_ENUMINPUT, &v2input) >= 0) {
            if (v2input.index) {
                info += "\n";
            }
            sprintf(buffer, "Input   : %d\n", v2input.index);
            info += buffer;
            sprintf(buffer, "Name    : %s\n", v2input.name);
            info += buffer;
            sprintf(buffer, "Type    : 0x%08X\n", v2input.type);
            info += buffer;
            sprintf(buffer, "Audioset: 0x%08X\n", v2input.audioset);
            info += buffer;
            sprintf(buffer, "Tuner   : 0x%08X\n", v2input.tuner);
            info += buffer;
            sprintf(buffer, "Standard: 0x%016llX ( ", (unsigned long long) v2input.std);
            info += buffer;
            if (v2input.std & 0x000FFF)
                info += "PAL ";
            if (v2input.std & 0x00F000)
                info += "NTSC ";
            if (v2input.std & 0x7F0000)
                info += "SECAM ";
            info += ")\n";
            sprintf(buffer, "Status  : %d\n", v2input.status);
            info += buffer;
            v2input.index++;
        }
        v2audio.index = 0;
        info += "Audio inputs:\n";
        while (ioctl(fd, VIDIOC_ENUMAUDIO, &v2audio) >= 0) {
            if (v2audio.index) {
                info += "\n";
            }
            sprintf(buffer, "Input   : %d\n", v2audio.index);
            info += buffer;
            sprintf(buffer, "Name    : %s\n", v2audio.name);
            info += buffer;
            v2audio.index++;
        }
        break;
    }
    return anytovariant(info.c_str());
}


Variant TVObject::m_Open(int numargs, Variant args[])
{
    TVOpen();
    return VARNULL;
}

Variant TVObject::m_Close(int numargs, Variant args[])
{
    TVClose();
    return VARNULL;
}

Variant TVObject::m_Next(int numargs, Variant args[])
{
    channel++;
    if (!channels[chanlist].freqs[channel].name) {
        channel = 0;
    }
    TVSetChannel();
    return VARNULL;
}

Variant TVObject::m_Prev(int numargs, Variant args[])
{
    channel--;
    if (channel < 0) {
        for (channel = 0; channels[chanlist].freqs[channel].name; channel++);
        channel--;
    }
    TVSetChannel();
    return VARNULL;
}

// Returns real frequency number already divided by 62.5 Khz or 62.2Hz
Variant TVObject::m_GetFrequency(int numargs, Variant args[])
{
    int freq = args[0];
    switch (version) {
    case 1:
        freq = freq * 16;
        if (!(flags & VIDEO_TUNER_LOW)) {
            freq /= 1000;
        }
        break;
            
    case 2:
        freq = freq * 16;
        if (!(flags & V4L2_TUNER_CAP_LOW)) {
            freq /= 1000;
        }
    }
    return anytovariant(freq);
}

// Returns frequency by channel
Variant TVObject::m_GetChannel(int numargs, Variant args[])
{
    const char *chan = args[0];
    for (int i = 0; channels[chanlist].freqs[i].name; i++) {
        if (!strcmp(chan, channels[chanlist].freqs[i].name)) {
            return anytovariant(channels[chanlist].freqs[i].freq);
        }
    }
    return VARNULL;
}

Variant TVObject::m_Mixer(int numargs, Variant args[])
{
    if (!strcmp(args[0], "set")) {
        TVSetMixer(args[1], args[2]);
    } else
    if (!strcmp(args[0], "get")) {
        return anytovariant((int) TVGetMixer(args[1]));
    } else
    if (!strcmp(args[0], "unmute")) {
        TVSetMixer("Unmute:Line", (int) args[1] ? 1 : 0);
    }
    return VARNULL;
}

