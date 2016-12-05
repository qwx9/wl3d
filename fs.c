#include <u.h>
#include <libc.h>
#include <bio.h>
#include "dat.h"
#include "fns.h"

int drofs;
u32int *pal, pals[Cend][256]={ {
	0x000000, 0x0000aa, 0x00aa00, 0x00aaaa, 0xaa0000, 0xaa00aa, 0xaa5500,
	0xaaaaaa, 0x555555, 0x5555ff, 0x55ff55, 0x55ffff, 0xff5555, 0xff55ff,
	0xffff55, 0xffffff, 0xeeeeee, 0xdedede, 0xd2d2d2, 0xc2c2c2, 0xb6b6b6,
	0xaaaaaa, 0x999999, 0x8d8d8d, 0x7d7d7d, 0x717171, 0x656565, 0x555555,
	0x484848, 0x383838, 0x2c2c2c, 0x202020, 0xff0000, 0xee0000, 0xe20000,
	0xd60000, 0xca0000, 0xbe0000, 0xb20000, 0xa50000, 0x990000, 0x890000,
	0x7d0000, 0x710000, 0x650000, 0x590000, 0x4c0000, 0x400000, 0xffdada,
	0xffbaba, 0xff9d9d, 0xff7d7d, 0xff5d5d, 0xff4040, 0xff2020, 0xff0000,
	0xffaa5d, 0xff9940, 0xff8920, 0xff7900, 0xe66d00, 0xce6100, 0xb65500,
	0x9d4c00, 0xffffda, 0xffffba, 0xffff9d, 0xffff7d, 0xfffa5d, 0xfff640,
	0xfff620, 0xfff600, 0xe6da00, 0xcec600, 0xb6ae00, 0x9d9d00, 0x858500,
	0x716d00, 0x595500, 0x404000, 0xd2ff5d, 0xc6ff40, 0xb6ff20, 0xa1ff00,
	0x91e600, 0x81ce00, 0x75b600, 0x619d00, 0xdaffda, 0xbeffba, 0x9dff9d,
	0x81ff7d, 0x61ff5d, 0x40ff40, 0x20ff20, 0x00ff00, 0x00ff00, 0x00ee00,
	0x00e200, 0x00d600, 0x04ca00, 0x04be00, 0x04b200, 0x04a500, 0x049900,
	0x048900, 0x047d00, 0x047100, 0x046500, 0x045900, 0x044c00, 0x044000,
	0xdaffff, 0xbaffff, 0x9dffff, 0x7dfffa, 0x5dffff, 0x40ffff, 0x20ffff,
	0x00ffff, 0x00e6e6, 0x00cece, 0x00b6b6, 0x009d9d, 0x008585, 0x007171,
	0x005959, 0x004040, 0x5dbeff, 0x40b2ff, 0x20aaff, 0x009dff, 0x008de6,
	0x007dce, 0x006db6, 0x005d9d, 0xdadaff, 0xbabeff, 0x9d9dff, 0x7d81ff,
	0x5d61ff, 0x4040ff, 0x2024ff, 0x0004ff, 0x0000ff, 0x0000ee, 0x0000e2,
	0x0000d6, 0x0000ca, 0x0000be, 0x0000b2, 0x0000a5, 0x000099, 0x000089,
	0x00007d, 0x000071, 0x000065, 0x000059, 0x00004c, 0x000040, 0x282828,
	0xffe234, 0xffd624, 0xffce18, 0xffc208, 0xffb600, 0xb620ff, 0xaa00ff,
	0x9900e6, 0x8100ce, 0x7500b6, 0x61009d, 0x500085, 0x440071, 0x340059,
	0x280040, 0xffdaff, 0xffbaff, 0xff9dff, 0xff7dff, 0xff5dff, 0xff40ff,
	0xff20ff, 0xff00ff, 0xe200e6, 0xca00ce, 0xb600b6, 0x9d009d, 0x850085,
	0x6d0071, 0x590059, 0x400040, 0xffeade, 0xffe2d2, 0xffdac6, 0xffd6be,
	0xffceb2, 0xffc6a5, 0xffbe9d, 0xffba91, 0xffb281, 0xffa571, 0xff9d61,
	0xf2955d, 0xea8d59, 0xde8955, 0xd28150, 0xca7d4c, 0xbe7948, 0xb67144,
	0xaa6940, 0xa1653c, 0x9d6138, 0x915d34, 0x895930, 0x81502c, 0x754c28,
	0x6d4824, 0x5d4020, 0x553c1c, 0x483818, 0x403018, 0x382c14, 0x28200c,
	0x610065, 0x006565, 0x006161, 0x00001c, 0x00002c, 0x302410, 0x480048,
	0x500050, 0x000034, 0x1c1c1c, 0x4c4c4c, 0x5d5d5d, 0x404040, 0x303030,
	0x343434, 0xdaf6f6, 0xbaeaea, 0x9ddede, 0x75caca, 0x48c2c2, 0x20b6b6,
	0x20b2b2, 0x00a5a5, 0x009999, 0x008d8d, 0x008585, 0x007d7d, 0x007979,
	0x007575, 0x007171, 0x006d6d, 0x990089
	},{
	0x200000, 0x200095, 0x209500, 0x209595, 0xb20000, 0xb20095, 0xb24c00,
	0xb29595, 0x694c4c, 0x694ce2, 0x69e24c, 0x69e2e2, 0xff4c4c, 0xff4ce2,
	0xffe24c, 0xffe2e2, 0xeed2d2, 0xe2c6c6, 0xd6baba, 0xcaaaaa, 0xbea1a1,
	0xb29595, 0xa58989, 0x997d7d, 0x8d7171, 0x816565, 0x755959, 0x694c4c,
	0x5d4040, 0x503434, 0x442828, 0x3c1c1c, 0xff0000, 0xee0000, 0xe60000,
	0xda0000, 0xce0000, 0xc60000, 0xba0000, 0xae0000, 0xa50000, 0x950000,
	0x8d0000, 0x810000, 0x750000, 0x6d0000, 0x610000, 0x590000, 0xffc2c2,
	0xffa5a5, 0xff8d8d, 0xff7171, 0xff5555, 0xff3838, 0xff1c1c, 0xff0000,
	0xff9555, 0xff8938, 0xff791c, 0xff6d00, 0xe66100, 0xd25500, 0xbe4c00,
	0xaa4400, 0xffe2c2, 0xffe2a5, 0xffe28d, 0xffe271, 0xffde55, 0xffda38,
	0xffda1c, 0xffda00, 0xe6c200, 0xd2ae00, 0xbe9900, 0xaa8d00, 0x917500,
	0x816100, 0x6d4c00, 0x593800, 0xd6e255, 0xcae238, 0xbee21c, 0xaee200,
	0x9dca00, 0x91b600, 0x85a100, 0x758d00, 0xdee2c2, 0xc6e2a5, 0xaae28d,
	0x91e271, 0x75e255, 0x59e238, 0x3ce21c, 0x20e200, 0x20e200, 0x20d200,
	0x20c600, 0x20be00, 0x20b200, 0x20aa00, 0x209d00, 0x209100, 0x208900,
	0x207900, 0x207100, 0x206500, 0x205900, 0x205000, 0x204400, 0x203800,
	0xdee2e2, 0xc2e2e2, 0xaae2e2, 0x8de2de, 0x71e2e2, 0x59e2e2, 0x3ce2e2,
	0x20e2e2, 0x20caca, 0x20b6b6, 0x20a1a1, 0x208d8d, 0x207575, 0x206565,
	0x205050, 0x203838, 0x71aae2, 0x599de2, 0x3c95e2, 0x208de2, 0x207dca,
	0x2071b6, 0x2061a1, 0x20558d, 0xdec2e2, 0xc2aae2, 0xaa8de2, 0x8d71e2,
	0x7155e2, 0x5938e2, 0x3c20e2, 0x2004e2, 0x2000e2, 0x2000d2, 0x2000c6,
	0x2000be, 0x2000b2, 0x2000aa, 0x20009d, 0x200091, 0x200089, 0x200079,
	0x200071, 0x200065, 0x200059, 0x200050, 0x200044, 0x200038, 0x402424,
	0xffc630, 0xffbe20, 0xffb618, 0xffaa08, 0xffa100, 0xbe1ce2, 0xb200e2,
	0xa500ca, 0x9100b6, 0x8500a1, 0x75008d, 0x650075, 0x590065, 0x4c0050,
	0x400038, 0xffc2e2, 0xffa5e2, 0xff8de2, 0xff71e2, 0xff55e2, 0xff38e2,
	0xff1ce2, 0xff00e2, 0xe600ca, 0xce00b6, 0xbe00a1, 0xaa008d, 0x910075,
	0x7d0065, 0x6d0050, 0x590038, 0xffcec6, 0xffc6ba, 0xffc2ae, 0xffbeaa,
	0xffb69d, 0xffae91, 0xffaa8d, 0xffa581, 0xff9d71, 0xff9165, 0xff8d55,
	0xf28555, 0xea7d50, 0xe2794c, 0xd67148, 0xce7144, 0xc66d40, 0xbe653c,
	0xb25d38, 0xae5938, 0xaa5534, 0x9d5530, 0x95502c, 0x914828, 0x854424,
	0x7d4020, 0x71381c, 0x69381c, 0x5d3418, 0x592c18, 0x502814, 0x401c0c,
	0x750059, 0x205959, 0x205555, 0x20001c, 0x200028, 0x482010, 0x5d0040,
	0x650048, 0x200030, 0x381c1c, 0x614444, 0x715555, 0x593838, 0x482c2c,
	0x4c3030, 0xdedada, 0xc2cece, 0xaac6c6, 0x85b2b2, 0x5daaaa, 0x3ca1a1,
	0x3c9d9d, 0x209191, 0x208989, 0x207d7d, 0x207575, 0x207171, 0x206d6d,
	0x206969, 0x206565, 0x206161, 0xa50079
	},{
	0x400000, 0x400081, 0x408100, 0x408181, 0xbe0000, 0xbe0081, 0xbe4000,
	0xbe8181, 0x7d4040, 0x7d40c2, 0x7dc240, 0x7dc2c2, 0xff4040, 0xff40c2,
	0xffc240, 0xffc2c2, 0xf2b6b6, 0xe6aaaa, 0xde9d9d, 0xd29191, 0xc68989,
	0xbe8181, 0xb27575, 0xaa6d6d, 0x9d6161, 0x955555, 0x894c4c, 0x7d4040,
	0x753838, 0x692c2c, 0x612424, 0x591818, 0xff0000, 0xf20000, 0xea0000,
	0xde0000, 0xd60000, 0xce0000, 0xc60000, 0xba0000, 0xb20000, 0xa50000,
	0x9d0000, 0x950000, 0x890000, 0x810000, 0x790000, 0x710000, 0xffa5a5,
	0xff8d8d, 0xff7979, 0xff6161, 0xff4848, 0xff3030, 0xff1818, 0xff0000,
	0xff8148, 0xff7530, 0xff6918, 0xff5d00, 0xea5500, 0xda4800, 0xc64000,
	0xb63c00, 0xffc2a5, 0xffc28d, 0xffc279, 0xffc261, 0xffbe48, 0xffba30,
	0xffba18, 0xffba00, 0xeaa500, 0xda9500, 0xc68500, 0xb67900, 0xa16500,
	0x955500, 0x814000, 0x713000, 0xdec248, 0xd2c230, 0xc6c218, 0xbac200,
	0xaeae00, 0xa19d00, 0x958900, 0x897900, 0xe2c2a5, 0xcec28d, 0xb6c279,
	0xa1c261, 0x89c248, 0x71c230, 0x59c218, 0x40c200, 0x40c200, 0x40b600,
	0x40aa00, 0x40a100, 0x409900, 0x409100, 0x408500, 0x407d00, 0x407500,
	0x406900, 0x406100, 0x405500, 0x404c00, 0x404400, 0x403c00, 0x403000,
	0xe2c2c2, 0xcac2c2, 0xb6c2c2, 0x9dc2be, 0x85c2c2, 0x71c2c2, 0x59c2c2,
	0x40c2c2, 0x40aeae, 0x409d9d, 0x408989, 0x407979, 0x406565, 0x405555,
	0x404444, 0x403030, 0x8591c2, 0x7185c2, 0x5981c2, 0x4079c2, 0x406dae,
	0x40619d, 0x405589, 0x404879, 0xe2a5c2, 0xca91c2, 0xb679c2, 0x9d61c2,
	0x8548c2, 0x7130c2, 0x591cc2, 0x4004c2, 0x4000c2, 0x4000b6, 0x4000aa,
	0x4000a1, 0x400099, 0x400091, 0x400085, 0x40007d, 0x400075, 0x400069,
	0x400061, 0x400055, 0x40004c, 0x400044, 0x40003c, 0x400030, 0x5d2020,
	0xffaa28, 0xffa11c, 0xff9d14, 0xff9108, 0xff8900, 0xc618c2, 0xbe00c2,
	0xb200ae, 0xa1009d, 0x950089, 0x890079, 0x7d0065, 0x710055, 0x650044,
	0x5d0030, 0xffa5c2, 0xff8dc2, 0xff79c2, 0xff61c2, 0xff48c2, 0xff30c2,
	0xff18c2, 0xff00c2, 0xea00ae, 0xd6009d, 0xc60089, 0xb60079, 0xa10065,
	0x910055, 0x810044, 0x710030, 0xffb2aa, 0xffaa9d, 0xffa595, 0xffa191,
	0xff9d85, 0xff957d, 0xff9179, 0xff8d6d, 0xff8561, 0xff7d55, 0xff7948,
	0xf67148, 0xee6d44, 0xe66940, 0xde613c, 0xd6613c, 0xce5d38, 0xc65534,
	0xbe5030, 0xba4c30, 0xb6482c, 0xae4828, 0xa54424, 0xa13c24, 0x953c20,
	0x91381c, 0x853018, 0x7d3018, 0x752c14, 0x712414, 0x692410, 0x5d180c,
	0x89004c, 0x404c4c, 0x404848, 0x400018, 0x400024, 0x651c0c, 0x750038,
	0x7d003c, 0x400028, 0x551818, 0x793c3c, 0x854848, 0x713030, 0x652424,
	0x652828, 0xe2baba, 0xcab2b2, 0xb6aaaa, 0x959999, 0x759191, 0x598989,
	0x598585, 0x407d7d, 0x407575, 0x406d6d, 0x406565, 0x406161, 0x405d5d,
	0x405959, 0x405555, 0x405555, 0xb20069
	},{
	0x610000, 0x61006d, 0x616d00, 0x616d6d, 0xca0000, 0xca006d, 0xca3800,
	0xca6d6d, 0x953838, 0x9538a1, 0x95a138, 0x95a1a1, 0xff3838, 0xff38a1,
	0xffa138, 0xffa1a1, 0xf29595, 0xea8d8d, 0xe28585, 0xda7979, 0xd27575,
	0xca6d6d, 0xbe6161, 0xb65959, 0xae5050, 0xa54848, 0x9d4040, 0x953838,
	0x8d3030, 0x812424, 0x791c1c, 0x751414, 0xff0000, 0xf20000, 0xee0000,
	0xe60000, 0xde0000, 0xd60000, 0xce0000, 0xc60000, 0xbe0000, 0xb60000,
	0xae0000, 0xa50000, 0x9d0000, 0x950000, 0x8d0000, 0x890000, 0xff8989,
	0xff7575, 0xff6565, 0xff5050, 0xff3c3c, 0xff2828, 0xff1414, 0xff0000,
	0xff6d3c, 0xff6128, 0xff5914, 0xff4c00, 0xee4400, 0xde3c00, 0xd23800,
	0xc23000, 0xffa189, 0xffa175, 0xffa165, 0xffa150, 0xff9d3c, 0xff9d28,
	0xff9d14, 0xff9d00, 0xee8900, 0xde7d00, 0xd26d00, 0xc26500, 0xb25500,
	0xa54400, 0x953800, 0x892800, 0xe2a13c, 0xdaa128, 0xd2a114, 0xc6a100,
	0xba9100, 0xb28100, 0xaa7500, 0x9d6500, 0xe6a189, 0xd6a175, 0xc2a165,
	0xb2a150, 0x9da13c, 0x89a128, 0x75a114, 0x61a100, 0x61a100, 0x619500,
	0x618d00, 0x618900, 0x618100, 0x617900, 0x617100, 0x616900, 0x616100,
	0x615900, 0x615000, 0x614800, 0x614000, 0x613800, 0x613000, 0x612800,
	0xe6a1a1, 0xd2a1a1, 0xc2a1a1, 0xaea19d, 0x99a1a1, 0x89a1a1, 0x75a1a1,
	0x61a1a1, 0x619191, 0x618181, 0x617575, 0x616565, 0x615555, 0x614848,
	0x613838, 0x612828, 0x9979a1, 0x8971a1, 0x756da1, 0x6165a1, 0x615991,
	0x615081, 0x614475, 0x613c65, 0xe689a1, 0xd279a1, 0xc265a1, 0xae50a1,
	0x993ca1, 0x8928a1, 0x7518a1, 0x6104a1, 0x6100a1, 0x610095, 0x61008d,
	0x610089, 0x610081, 0x610079, 0x610071, 0x610069, 0x610061, 0x610059,
	0x610050, 0x610048, 0x610040, 0x610038, 0x610030, 0x610028, 0x791c1c,
	0xff8d24, 0xff8918, 0xff8110, 0xff7908, 0xff7500, 0xd214a1, 0xca00a1,
	0xbe0091, 0xb20081, 0xaa0075, 0x9d0065, 0x910055, 0x890048, 0x810038,
	0x790028, 0xff89a1, 0xff75a1, 0xff65a1, 0xff50a1, 0xff3ca1, 0xff28a1,
	0xff14a1, 0xff00a1, 0xee0091, 0xde0081, 0xd20075, 0xc20065, 0xb20055,
	0xa10048, 0x950038, 0x890028, 0xff958d, 0xff8d85, 0xff897d, 0xff8979,
	0xff8171, 0xff7d69, 0xff7965, 0xff755d, 0xff7150, 0xff6948, 0xff653c,
	0xf6613c, 0xf25938, 0xea5938, 0xe25034, 0xde5030, 0xd64c30, 0xd2482c,
	0xca4428, 0xc64028, 0xc23c24, 0xba3c24, 0xb63820, 0xb2341c, 0xaa301c,
	0xa13018, 0x992814, 0x952814, 0x8d2410, 0x892010, 0x811c10, 0x791408,
	0x9d0040, 0x614040, 0x613c3c, 0x610014, 0x61001c, 0x7d180c, 0x8d0030,
	0x910034, 0x610024, 0x711414, 0x8d3030, 0x993c3c, 0x892828, 0x7d2020,
	0x812424, 0xe69d9d, 0xd29595, 0xc28d8d, 0xaa8181, 0x8d7979, 0x757575,
	0x757171, 0x616969, 0x616161, 0x615959, 0x615555, 0x615050, 0x614c4c,
	0x614c4c, 0x614848, 0x614444, 0xbe0059
	},{
	0x810000, 0x810055, 0x815500, 0x815555, 0xd60000, 0xd60055, 0xd62c00,
	0xd65555, 0xaa2c2c, 0xaa2c81, 0xaa812c, 0xaa8181, 0xff2c2c, 0xff2c81,
	0xff812c, 0xff8181, 0xf67979, 0xee7171, 0xea6969, 0xe26161, 0xda5d5d,
	0xd65555, 0xce4c4c, 0xc64848, 0xbe4040, 0xba3838, 0xb23434, 0xaa2c2c,
	0xa52424, 0x9d1c1c, 0x951818, 0x911010, 0xff0000, 0xf60000, 0xf20000,
	0xea0000, 0xe60000, 0xde0000, 0xda0000, 0xd20000, 0xce0000, 0xc60000,
	0xbe0000, 0xba0000, 0xb20000, 0xae0000, 0xa50000, 0xa10000, 0xff6d6d,
	0xff5d5d, 0xff5050, 0xff4040, 0xff3030, 0xff2020, 0xff1010, 0xff0000,
	0xff5530, 0xff4c20, 0xff4410, 0xff3c00, 0xf23800, 0xe63000, 0xda2c00,
	0xce2800, 0xff816d, 0xff815d, 0xff8150, 0xff8140, 0xff7d30, 0xff7d20,
	0xff7d10, 0xff7d00, 0xf26d00, 0xe66500, 0xda5900, 0xce5000, 0xc24400,
	0xba3800, 0xae2c00, 0xa12000, 0xea8130, 0xe28120, 0xda8110, 0xd28100,
	0xca7500, 0xc26900, 0xba5d00, 0xb25000, 0xee816d, 0xde815d, 0xce8150,
	0xc28140, 0xb28130, 0xa18120, 0x918110, 0x818100, 0x818100, 0x817900,
	0x817100, 0x816d00, 0x816500, 0x816100, 0x815900, 0x815500, 0x814c00,
	0x814400, 0x814000, 0x813800, 0x813400, 0x812c00, 0x812800, 0x812000,
	0xee8181, 0xde8181, 0xce8181, 0xbe817d, 0xae8181, 0xa18181, 0x918181,
	0x818181, 0x817575, 0x816969, 0x815d5d, 0x815050, 0x814444, 0x813838,
	0x812c2c, 0x812020, 0xae6181, 0xa15981, 0x915581, 0x815081, 0x814875,
	0x814069, 0x81385d, 0x813050, 0xee6d81, 0xde6181, 0xce5081, 0xbe4081,
	0xae3081, 0xa12081, 0x911481, 0x810481, 0x810081, 0x810079, 0x810071,
	0x81006d, 0x810065, 0x810061, 0x810059, 0x810055, 0x81004c, 0x810044,
	0x810040, 0x810038, 0x810034, 0x81002c, 0x810028, 0x810020, 0x951414,
	0xff711c, 0xff6d14, 0xff690c, 0xff6104, 0xff5d00, 0xda1081, 0xd60081,
	0xce0075, 0xc20069, 0xba005d, 0xb20050, 0xaa0044, 0xa10038, 0x99002c,
	0x950020, 0xff6d81, 0xff5d81, 0xff5081, 0xff4081, 0xff3081, 0xff2081,
	0xff1081, 0xff0081, 0xf20075, 0xe60069, 0xda005d, 0xce0050, 0xc20044,
	0xb60038, 0xae002c, 0xa10020, 0xff7571, 0xff7169, 0xff6d65, 0xff6d61,
	0xff6959, 0xff6555, 0xff6150, 0xff5d48, 0xff5940, 0xff5538, 0xff5030,
	0xfa4c30, 0xf6482c, 0xee442c, 0xea4028, 0xe64028, 0xde3c24, 0xda3824,
	0xd63420, 0xd23420, 0xce301c, 0xca301c, 0xc62c18, 0xc22818, 0xba2814,
	0xb62414, 0xae2010, 0xaa2010, 0xa51c0c, 0xa1180c, 0x9d180c, 0x951008,
	0xb20034, 0x813434, 0x813030, 0x810010, 0x810018, 0x991408, 0xa50024,
	0xaa0028, 0x81001c, 0x8d1010, 0xa52828, 0xae3030, 0xa12020, 0x991818,
	0x991c1c, 0xee7d7d, 0xde7575, 0xce7171, 0xba6565, 0xa56161, 0x915d5d,
	0x915959, 0x815555, 0x814c4c, 0x814848, 0x814444, 0x814040, 0x813c3c,
	0x813c3c, 0x813838, 0x813838, 0xce0044
	},{
	0xa10000, 0xa10040, 0xa14000, 0xa14040, 0xde0000, 0xde0040, 0xde2000,
	0xde4040, 0xbe2020, 0xbe2061, 0xbe6120, 0xbe6161, 0xff2020, 0xff2061,
	0xff6120, 0xff6161, 0xfa5d5d, 0xf25555, 0xee5050, 0xea4848, 0xe24444,
	0xde4040, 0xda3c3c, 0xd63838, 0xce3030, 0xca2c2c, 0xc62828, 0xbe2020,
	0xba1c1c, 0xb61818, 0xb21414, 0xae0c0c, 0xff0000, 0xfa0000, 0xf60000,
	0xee0000, 0xea0000, 0xe60000, 0xe20000, 0xde0000, 0xda0000, 0xd20000,
	0xce0000, 0xca0000, 0xc60000, 0xc20000, 0xbe0000, 0xba0000, 0xff5555,
	0xff4848, 0xff3c3c, 0xff3030, 0xff2424, 0xff1818, 0xff0c0c, 0xff0000,
	0xff4024, 0xff3c18, 0xff340c, 0xff3000, 0xf62c00, 0xee2400, 0xe22000,
	0xda2000, 0xff6155, 0xff6148, 0xff613c, 0xff6130, 0xff6124, 0xff5d18,
	0xff5d0c, 0xff5d00, 0xf65500, 0xee4c00, 0xe24400, 0xda3c00, 0xd23400,
	0xca2c00, 0xc22000, 0xba1800, 0xee6124, 0xea6118, 0xe2610c, 0xde6100,
	0xd65900, 0xd25000, 0xca4400, 0xc63c00, 0xf26155, 0xe66148, 0xda613c,
	0xd26130, 0xc66124, 0xba6118, 0xae610c, 0xa16100, 0xa16100, 0xa15d00,
	0xa15500, 0xa15000, 0xa14c00, 0xa14800, 0xa14400, 0xa14000, 0xa13c00,
	0xa13400, 0xa13000, 0xa12c00, 0xa12800, 0xa12400, 0xa12000, 0xa11800,
	0xf26161, 0xe66161, 0xda6161, 0xce6161, 0xc26161, 0xba6161, 0xae6161,
	0xa16161, 0xa15959, 0xa15050, 0xa14444, 0xa13c3c, 0xa13434, 0xa12c2c,
	0xa12424, 0xa11818, 0xc24861, 0xba4461, 0xae4061, 0xa13c61, 0xa13859,
	0xa13050, 0xa12c44, 0xa1243c, 0xf25561, 0xe64861, 0xda3c61, 0xce3061,
	0xc22461, 0xba1861, 0xae1061, 0xa10461, 0xa10061, 0xa1005d, 0xa10055,
	0xa10050, 0xa1004c, 0xa10048, 0xa10044, 0xa10040, 0xa1003c, 0xa10034,
	0xa10030, 0xa1002c, 0xa10028, 0xa10024, 0xa10020, 0xa10018, 0xae1010,
	0xff5514, 0xff5010, 0xff500c, 0xff4804, 0xff4400, 0xe20c61, 0xde0061,
	0xda0059, 0xd20050, 0xca0044, 0xc6003c, 0xbe0034, 0xba002c, 0xb20024,
	0xae0018, 0xff5561, 0xff4861, 0xff3c61, 0xff3061, 0xff2461, 0xff1861,
	0xff0c61, 0xff0061, 0xf60059, 0xea0050, 0xe20044, 0xda003c, 0xd20034,
	0xca002c, 0xc20024, 0xba0018, 0xff5955, 0xff5550, 0xff554c, 0xff5048,
	0xff5044, 0xff4c40, 0xff483c, 0xff4838, 0xff4430, 0xff402c, 0xff3c24,
	0xfa3824, 0xf63824, 0xf23420, 0xee3020, 0xea3020, 0xe6301c, 0xe22c1c,
	0xde2818, 0xde2818, 0xda2418, 0xd62414, 0xd22414, 0xd22014, 0xca2010,
	0xca1c10, 0xc2180c, 0xbe180c, 0xba180c, 0xba140c, 0xb61408, 0xae0c08,
	0xc60028, 0xa12828, 0xa12424, 0xa1000c, 0xa10014, 0xb21008, 0xba001c,
	0xbe0020, 0xa10014, 0xaa0c0c, 0xbe2020, 0xc22424, 0xba1818, 0xb21414,
	0xb21414, 0xf25d5d, 0xe65959, 0xda5555, 0xca4c4c, 0xba4848, 0xae4444,
	0xae4444, 0xa14040, 0xa13c3c, 0xa13838, 0xa13434, 0xa13030, 0xa13030,
	0xa12c2c, 0xa12c2c, 0xa12c2c, 0xda0034
	},{
	0xc20000, 0xc2002c, 0xc22c00, 0xc22c2c, 0xea0000, 0xea002c, 0xea1800,
	0xea2c2c, 0xd61818, 0xd61840, 0xd64018, 0xd64040, 0xff1818, 0xff1840,
	0xff4018, 0xff4040, 0xfa3c3c, 0xf63838, 0xf63434, 0xf23030, 0xee3030,
	0xea2c2c, 0xe62828, 0xe22424, 0xde2020, 0xde1c1c, 0xda1c1c, 0xd61818,
	0xd21414, 0xce1010, 0xca0c0c, 0xca0808, 0xff0000, 0xfa0000, 0xfa0000,
	0xf60000, 0xf20000, 0xee0000, 0xee0000, 0xea0000, 0xe60000, 0xe20000,
	0xde0000, 0xde0000, 0xda0000, 0xd60000, 0xd20000, 0xd20000, 0xff3838,
	0xff3030, 0xff2828, 0xff2020, 0xff1818, 0xff1010, 0xff0808, 0xff0000,
	0xff2c18, 0xff2810, 0xff2408, 0xff2000, 0xfa1c00, 0xf21800, 0xee1800,
	0xe61400, 0xff4038, 0xff4030, 0xff4028, 0xff4020, 0xff4018, 0xff4010,
	0xff4008, 0xff4000, 0xfa3800, 0xf23400, 0xee2c00, 0xe62800, 0xe22400,
	0xde1c00, 0xd61800, 0xd21000, 0xf64018, 0xf24010, 0xee4008, 0xea4000,
	0xe63c00, 0xe23400, 0xde3000, 0xda2800, 0xf64038, 0xee4030, 0xe64028,
	0xe24020, 0xda4018, 0xd24010, 0xca4008, 0xc24000, 0xc24000, 0xc23c00,
	0xc23800, 0xc23800, 0xc23400, 0xc23000, 0xc22c00, 0xc22c00, 0xc22800,
	0xc22400, 0xc22000, 0xc21c00, 0xc21c00, 0xc21800, 0xc21400, 0xc21000,
	0xf64040, 0xee4040, 0xe64040, 0xde4040, 0xd64040, 0xd24040, 0xca4040,
	0xc24040, 0xc23c3c, 0xc23434, 0xc23030, 0xc22828, 0xc22424, 0xc21c1c,
	0xc21818, 0xc21010, 0xd63040, 0xd22c40, 0xca2c40, 0xc22840, 0xc2243c,
	0xc22034, 0xc21c30, 0xc21828, 0xf63840, 0xee3040, 0xe62840, 0xde2040,
	0xd61840, 0xd21040, 0xca0c40, 0xc20440, 0xc20040, 0xc2003c, 0xc20038,
	0xc20038, 0xc20034, 0xc20030, 0xc2002c, 0xc2002c, 0xc20028, 0xc20024,
	0xc20020, 0xc2001c, 0xc2001c, 0xc20018, 0xc20014, 0xc20010, 0xca0c0c,
	0xff3810, 0xff380c, 0xff3408, 0xff3004, 0xff3000, 0xee0840, 0xea0040,
	0xe6003c, 0xe20034, 0xde0030, 0xda0028, 0xd60024, 0xd2001c, 0xce0018,
	0xca0010, 0xff3840, 0xff3040, 0xff2840, 0xff2040, 0xff1840, 0xff1040,
	0xff0840, 0xff0040, 0xfa003c, 0xf20034, 0xee0030, 0xe60028, 0xe20024,
	0xda001c, 0xd60018, 0xd20010, 0xff3c38, 0xff3834, 0xff3834, 0xff3830,
	0xff342c, 0xff342c, 0xff3028, 0xff3024, 0xff2c20, 0xff2c1c, 0xff2818,
	0xff2818, 0xfa2418, 0xf62418, 0xf62014, 0xf22014, 0xee2014, 0xee1c14,
	0xea1c10, 0xea1c10, 0xe61810, 0xe61810, 0xe2180c, 0xe2140c, 0xde140c,
	0xda140c, 0xd61008, 0xd61008, 0xd21008, 0xd20c08, 0xce0c08, 0xca0804,
	0xda001c, 0xc21c1c, 0xc21818, 0xc20008, 0xc2000c, 0xce0c04, 0xd20014,
	0xd60014, 0xc20010, 0xc60808, 0xd21414, 0xd61818, 0xd21010, 0xce0c0c,
	0xce1010, 0xf64040, 0xee3c3c, 0xe63838, 0xde3434, 0xd23030, 0xca3030,
	0xca2c2c, 0xc22c2c, 0xc22828, 0xc22424, 0xc22424, 0xc22020, 0xc22020,
	0xc22020, 0xc21c1c, 0xc21c1c, 0xe60024
	},{
	0x0c0c00, 0x0c0ca1, 0x0cae00, 0x0caea1, 0xae0c00, 0xae0ca1, 0xae5d00,
	0xaeaea1, 0x5d5d50, 0x5d5df2, 0x5dff50, 0x5dfff2, 0xff5d50, 0xff5df2,
	0xffff50, 0xfffff2, 0xeeeee6, 0xdeded6, 0xd2d2ca, 0xc2c2ba, 0xb6b6ae,
	0xaeaea1, 0x9d9d95, 0x919189, 0x818179, 0x75756d, 0x696961, 0x5d5d50,
	0x505048, 0x404038, 0x34342c, 0x282820, 0xff0c00, 0xee0c00, 0xe20c00,
	0xd60c00, 0xca0c00, 0xbe0c00, 0xb60c00, 0xaa0c00, 0x9d0c00, 0x8d0c00,
	0x810c00, 0x750c00, 0x690c00, 0x610c00, 0x550c00, 0x480c00, 0xffdad2,
	0xffbab2, 0xffa199, 0xff8179, 0xff6159, 0xff4840, 0xff2820, 0xff0c00,
	0xffae59, 0xff9d40, 0xff8d20, 0xff7d00, 0xe67100, 0xce6500, 0xb65d00,
	0xa15500, 0xffffd2, 0xffffb2, 0xffff99, 0xffff79, 0xfffa59, 0xfff640,
	0xfff620, 0xfff600, 0xe6da00, 0xcec600, 0xb6ae00, 0xa1a100, 0x898900,
	0x757100, 0x615d00, 0x484800, 0xd2ff59, 0xc6ff40, 0xb6ff20, 0xa5ff00,
	0x95e600, 0x85ce00, 0x79b600, 0x69a100, 0xdaffd2, 0xbeffb2, 0xa1ff99,
	0x85ff79, 0x69ff59, 0x48ff40, 0x28ff20, 0x0cff00, 0x0cff00, 0x0cee00,
	0x0ce200, 0x0cd600, 0x10ca00, 0x10be00, 0x10b200, 0x10aa00, 0x109d00,
	0x108d00, 0x108100, 0x107500, 0x106900, 0x106100, 0x105500, 0x104800,
	0xdafff2, 0xbafff2, 0xa1fff2, 0x81ffee, 0x65fff2, 0x48fff2, 0x28fff2,
	0x0cfff2, 0x0ce6de, 0x0ccec6, 0x0cb6ae, 0x0ca199, 0x0c8981, 0x0c756d,
	0x0c6155, 0x0c4840, 0x65bef2, 0x48b2f2, 0x28aef2, 0x0ca1f2, 0x0c91de,
	0x0c81c6, 0x0c71ae, 0x0c6199, 0xdadaf2, 0xbabef2, 0xa1a1f2, 0x8185f2,
	0x6565f2, 0x4848f2, 0x282cf2, 0x0c10f2, 0x0c0cf2, 0x0c0ce6, 0x0c0cda,
	0x0c0cce, 0x0c0cc2, 0x0c0cb6, 0x0c0caa, 0x0c0c9d, 0x0c0c95, 0x0c0c85,
	0x0c0c79, 0x0c0c6d, 0x0c0c61, 0x0c0c55, 0x0c0c4c, 0x0c0c40, 0x303028,
	0xffe234, 0xffd624, 0xffce18, 0xffc208, 0xffb600, 0xb628f2, 0xae0cf2,
	0x9d0cde, 0x850cc6, 0x790cae, 0x690c99, 0x590c81, 0x4c0c6d, 0x3c0c55,
	0x300c40, 0xffdaf2, 0xffbaf2, 0xffa1f2, 0xff81f2, 0xff61f2, 0xff48f2,
	0xff28f2, 0xff0cf2, 0xe20cde, 0xca0cc6, 0xb60cae, 0xa10c99, 0x890c81,
	0x710c6d, 0x610c55, 0x480c40, 0xffead6, 0xffe2ca, 0xffdabe, 0xffd6b6,
	0xffceaa, 0xffc69d, 0xffbe99, 0xffba8d, 0xffb27d, 0xffaa6d, 0xffa15d,
	0xf29959, 0xea9155, 0xde8d50, 0xd2854c, 0xca814c, 0xbe7d48, 0xb67544,
	0xae6d40, 0xa5693c, 0xa16538, 0x956134, 0x8d6130, 0x85592c, 0x795528,
	0x715024, 0x654820, 0x5d441c, 0x504018, 0x483818, 0x403414, 0x30280c,
	0x690c61, 0x0c6961, 0x0c655d, 0x0c0c1c, 0x0c0c2c, 0x382c10, 0x500c48,
	0x590c4c, 0x0c0c34, 0x24241c, 0x55554c, 0x656159, 0x484840, 0x383830,
	0x3c3c34, 0xdaf6ea, 0xbaeae2, 0xa1ded6, 0x79cac2, 0x50c2ba, 0x28b6ae,
	0x28b2aa, 0x0caa9d, 0x0c9d95, 0x0c9189, 0x0c8981, 0x0c8179, 0x0c7d75,
	0x0c7971, 0x0c756d, 0x0c7169, 0x9d0c85
	},{
	0x181800, 0x181899, 0x18b200, 0x18b299, 0xb21800, 0xb21899, 0xb26500,
	0xb2b299, 0x65654c, 0x6565e6, 0x65ff4c, 0x65ffe6, 0xff654c, 0xff65e6,
	0xffff4c, 0xffffe6, 0xeeeeda, 0xdedeca, 0xd6d6be, 0xc6c6b2, 0xbabaa5,
	0xb2b299, 0xa1a18d, 0x959581, 0x898971, 0x7d7d69, 0x71715d, 0x65654c,
	0x595944, 0x4c4834, 0x404028, 0x343420, 0xff1800, 0xee1800, 0xe21800,
	0xda1800, 0xce1800, 0xc21800, 0xba1800, 0xae1800, 0xa11800, 0x951800,
	0x891800, 0x7d1800, 0x711800, 0x691800, 0x5d1800, 0x501800, 0xffdac6,
	0xffbeaa, 0xffa591, 0xff8971, 0xff6955, 0xff503c, 0xff3420, 0xff1800,
	0xffb255, 0xffa13c, 0xff9120, 0xff8500, 0xe67900, 0xd26d00, 0xba6500,
	0xa55d00, 0xffffc6, 0xffffaa, 0xffff91, 0xffff71, 0xfffa55, 0xfff63c,
	0xfff620, 0xfff600, 0xe6da00, 0xd2ca00, 0xbab200, 0xa5a500, 0x918d00,
	0x7d7900, 0x696500, 0x505000, 0xd6ff55, 0xcaff3c, 0xbaff20, 0xaaff00,
	0x99e600, 0x8dd200, 0x81ba00, 0x71a500, 0xdeffc6, 0xc2ffaa, 0xa5ff91,
	0x8dff71, 0x71ff55, 0x50ff3c, 0x34ff20, 0x18ff00, 0x18ff00, 0x18ee00,
	0x18e200, 0x18d600, 0x1cce00, 0x1cc200, 0x1cb600, 0x1cae00, 0x1ca100,
	0x1c9100, 0x1c8900, 0x1c7d00, 0x1c7100, 0x1c6900, 0x1c5d00, 0x1c5000,
	0xdeffe6, 0xbeffe6, 0xa5ffe6, 0x89ffe2, 0x6dffe6, 0x50ffe6, 0x34ffe6,
	0x18ffe6, 0x18e6d2, 0x18d2ba, 0x18baa5, 0x18a591, 0x188d79, 0x187d69,
	0x186950, 0x18503c, 0x6dc2e6, 0x50b6e6, 0x34b2e6, 0x18a5e6, 0x1895d2,
	0x1889ba, 0x1879a5, 0x186991, 0xdedae6, 0xbec2e6, 0xa5a5e6, 0x898de6,
	0x6d6de6, 0x5050e6, 0x3438e6, 0x181ce6, 0x1818e6, 0x1818da, 0x1818ce,
	0x1818c2, 0x1818b6, 0x1818ae, 0x1818a1, 0x181895, 0x18188d, 0x18187d,
	0x181871, 0x181869, 0x18185d, 0x181850, 0x181848, 0x18183c, 0x3c3c24,
	0xffe230, 0xffd624, 0xffd218, 0xffc608, 0xffba00, 0xba34e6, 0xb218e6,
	0xa118d2, 0x8d18ba, 0x8118a5, 0x711891, 0x611879, 0x551869, 0x481850,
	0x3c183c, 0xffdae6, 0xffbee6, 0xffa5e6, 0xff89e6, 0xff69e6, 0xff50e6,
	0xff34e6, 0xff18e6, 0xe218d2, 0xce18ba, 0xba18a5, 0xa51891, 0x911879,
	0x791869, 0x691850, 0x50183c, 0xffeaca, 0xffe2be, 0xffdab6, 0xffd6ae,
	0xffd2a1, 0xffca95, 0xffc291, 0xffbe85, 0xffb675, 0xffae69, 0xffa559,
	0xf29d55, 0xea9550, 0xde914c, 0xd68d48, 0xce8948, 0xc28544, 0xba7d40,
	0xb2753c, 0xaa7138, 0xa56d34, 0x996930, 0x95692c, 0x8d6128, 0x815d24,
	0x795924, 0x6d5020, 0x654c1c, 0x594818, 0x504418, 0x4c4014, 0x3c340c,
	0x71185d, 0x18715d, 0x186d59, 0x18181c, 0x181828, 0x443810, 0x591844,
	0x611848, 0x181830, 0x30301c, 0x5d5d48, 0x6d6955, 0x50503c, 0x44442c,
	0x484430, 0xdef6de, 0xbeead6, 0xa5deca, 0x81ceb6, 0x59c6b2, 0x34baa5,
	0x34b6a1, 0x18ae95, 0x18a18d, 0x189581, 0x188d79, 0x188971, 0x18856d,
	0x18816d, 0x187d69, 0x187965, 0xa1187d
	},{
	0x242400, 0x242491, 0x24b600, 0x24b691, 0xb62400, 0xb62491, 0xb66d00,
	0xb6b691, 0x6d6d48, 0x6d6dda, 0x6dff48, 0x6dffda, 0xff6d48, 0xff6dda,
	0xffff48, 0xffffda, 0xeeeece, 0xe2e2be, 0xd6d6b6, 0xcacaa5, 0xbebe9d,
	0xb6b691, 0xa5a585, 0x9d9d79, 0x8d8d6d, 0x858561, 0x797959, 0x6d6d48,
	0x616140, 0x555530, 0x484828, 0x40401c, 0xff2400, 0xee2400, 0xe62400,
	0xda2400, 0xd22400, 0xc62400, 0xbe2400, 0xb22400, 0xa52400, 0x992400,
	0x8d2400, 0x852400, 0x792400, 0x712400, 0x652400, 0x5d2400, 0xffdeba,
	0xffc2a1, 0xffaa89, 0xff8d6d, 0xff7150, 0xff5938, 0xff401c, 0xff2400,
	0xffb650, 0xffa538, 0xff991c, 0xff8900, 0xea8100, 0xd27500, 0xbe6d00,
	0xaa6500, 0xffffba, 0xffffa1, 0xffff89, 0xffff6d, 0xfffa50, 0xfff638,
	0xfff61c, 0xfff600, 0xeade00, 0xd2ca00, 0xbeb600, 0xaaaa00, 0x959500,
	0x858100, 0x716d00, 0x5d5900, 0xd6ff50, 0xceff38, 0xbeff1c, 0xaeff00,
	0xa1e600, 0x91d200, 0x89be00, 0x79aa00, 0xdeffba, 0xc6ffa1, 0xaaff89,
	0x91ff6d, 0x79ff50, 0x5dff38, 0x40ff1c, 0x24ff00, 0x24ff00, 0x24ee00,
	0x24e200, 0x24da00, 0x28ce00, 0x28c600, 0x28ba00, 0x28b200, 0x28a500,
	0x289900, 0x288d00, 0x288500, 0x287900, 0x287100, 0x286500, 0x285900,
	0xdeffda, 0xc2ffda, 0xaaffda, 0x8dffd6, 0x75ffda, 0x5dffda, 0x40ffda,
	0x24ffda, 0x24e6c6, 0x24d2b2, 0x24be9d, 0x24aa89, 0x249575, 0x248561,
	0x24714c, 0x245938, 0x75c6da, 0x5dbada, 0x40b6da, 0x24aada, 0x249dc6,
	0x248db2, 0x24819d, 0x247189, 0xdededa, 0xc2c6da, 0xaaaada, 0x8d91da,
	0x7575da, 0x5d59da, 0x4040da, 0x2428da, 0x2424da, 0x2424ce, 0x2424c2,
	0x2424ba, 0x2424ae, 0x2424a1, 0x242499, 0x24248d, 0x242485, 0x242475,
	0x24246d, 0x242461, 0x242459, 0x24244c, 0x242444, 0x242438, 0x484424,
	0xffe230, 0xffda20, 0xffd218, 0xffca08, 0xffbe00, 0xbe40da, 0xb624da,
	0xa524c6, 0x9124b2, 0x89249d, 0x792489, 0x692475, 0x612461, 0x50244c,
	0x482438, 0xffdeda, 0xffc2da, 0xffaada, 0xff8dda, 0xff71da, 0xff59da,
	0xff40da, 0xff24da, 0xe624c6, 0xd224b2, 0xbe249d, 0xaa2489, 0x952475,
	0x812461, 0x71244c, 0x5d2438, 0xffeabe, 0xffe2b6, 0xffdeaa, 0xffdaa1,
	0xffd299, 0xffca8d, 0xffc689, 0xffc27d, 0xffba71, 0xffb261, 0xffaa55,
	0xf2a150, 0xea9d4c, 0xe29948, 0xd69144, 0xd28d44, 0xc68940, 0xbe853c,
	0xb67d38, 0xae7934, 0xaa7530, 0xa17130, 0x99712c, 0x916928, 0x896524,
	0x816120, 0x75591c, 0x6d5918, 0x615518, 0x5d4c18, 0x554814, 0x48400c,
	0x792459, 0x247959, 0x247555, 0x242418, 0x242428, 0x4c4010, 0x612440,
	0x692444, 0x242430, 0x3c3c18, 0x656544, 0x757150, 0x5d5938, 0x4c4c2c,
	0x505030, 0xdef6d2, 0xc2eaca, 0xaae2be, 0x89ceae, 0x61caa5, 0x40be9d,
	0x40ba99, 0x24b28d, 0x24a585, 0x249d79, 0x249575, 0x248d6d, 0x248969,
	0x248565, 0x248561, 0x24815d, 0xa52475
	}
};

static u32int sodpal[] = {
	0x003800, 0x002800,
	0x203400, 0x202400,
	0x402c00, 0x402000,
	0x612400, 0x611c00,
	0x811c00, 0x811400,
	0xa11800, 0xa11000,
	0xc21000, 0xc20c00,
	0x0c4000, 0x0c3000,
	0x184800, 0x183c00,
	0x245500, 0x244400
};

/* what bullshit. */
uchar *pict;
static uchar picts[4][Pend]={ {
	3, 15, 8, 10, 11, 24, 27, 34, 35, 23, 7, 12, 13, 14, 36, 16, 21, 25,
	26, 87, 37, 129, 40, 41, 42, 52, 79, 80, 81, 82, 84, 0, 0, 83, 85,
	86, 0, 0, 88, 92, 93, 94, 95, 96, 106, 127, 128, 0, 0, 0, 130, 131
	},{
	14, 27, 20, 22, 23, 36, 39, 46, 47, 35, 19, 24, 25, 26, 48, 28, 33,
	37, 38, 99, 49, 141, 52, 53, 54, 64, 91, 92, 93, 94, 96, 0, 0, 95,
	97, 98, 0, 0, 100, 104, 105, 106, 107, 108, 118, 139, 140, 0, 0, 0, 142,
	143
	},{
	0, 1, 2, 4, 5, 6, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 22, 24, 25,
	26, 27, 0, 28, 29, 30, 40, 67, 68, 69, 70, 71, 72, 0, 73, 74, 75, 0,
	0, 76, 80, 81, 82, 83, 84, 94, 115, 116, 117, 120, 122, 123, 124
	},{
 	0, 1, 2, 4, 5, 6, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 22, 24, 25,
	26, 27, 28, 33, 34, 35, 45, 72, 73, 74, 75, 76, 77, 78, 87, 88, 89,
	90, 91, 98, 102, 103, 104, 105, 106, 116, 137, 138, 139, 142, 144, 145,
	146
	}
};
static uchar pcmt[2][46]={ {
	Sgd, Sdog, Sclosedoor, Sopendoor, Smg, Spistol, Sgatling, Sss, Shans,
	Shansdie, Shansfire, Sssfire, Sscream1, Sscream2, Send, Spushwall,
	Sdogdie, Smutdie, Shitlerdie, Seva, Sssdie, Sgdfire, Sslurp,
	Sfake, Sschbdie, Sschb, Shitler, Sofc, Sofcdie, Sdogfire, Slvlend,
	Smechwalk, Syeah, Smechdie, Sscream4, Sscream5, Sottodie, Sotto,
	Sfett, Sscream6, Sscream7, Sscream8, Sscream9, Sgretel, Sgreteldie,
	Sfettdie
	},{
	Sgd, Sdog, Sclosedoor, Sopendoor, Smg, Spistol, Sgatling, Sss,
	Shansfire, Sssfire, Sscream1, Sscream2, Send, Spushwall, Sdogdie,
	Smutdie, Sssdie, Sgdfire, Sslurp, Sofc, Sofcdie, Sdogfire,
	Slvlend, Sscream4, Sscream5, Sscream6, Sscream7, Sscream8, Sscream9,
	Strans, Stransdie, Swilh, Swilhdie, Suberdie, Sknight,
	Sknightdie, Sangel, Sangeldie, Sgetgatling, Sspear
	}
};

enum{
	Nplane = 2,
	Planesz = Mapa * Nplane,
	Mapsz = Planesz * Nplane
};
static Dat *pcms;
static int alofs, npcm;
static u16int rlewtag;
static u32int *mapofs, *mape;

#define	GBIT16(p)	((p)[0]|((p)[1]<<8))

static Biobuf *
bopen(char *f, int m)
{
	char s[16];
	Biobuf *bf;

	snprint(s, sizeof s, "%s%s", f, ext);
	bf = Bopen(s, m);
	if(bf == nil)
		sysfatal("bopen: %r");
	Blethal(bf, nil);
	return bf;
}

static long
eread(Biobuf *bf, void *u, long n)
{
	if(Bread(bf, u, n) != n)
		sysfatal("eread: short read: %r");
	return n;
}

static u8int
get8(Biobuf *bf)
{
	u8int v;

	eread(bf, &v, 1);
	return v;
}

static u16int
get16(Biobuf *bf)
{
	u16int v;

	v = get8(bf);
	return v | get8(bf)<<8;
}

static u32int
get24(Biobuf *bf)
{
	u32int v;

	v = get16(bf);
	return v | get8(bf)<<16;
}

static u32int
get32(Biobuf *bf)
{
	u32int v;

	v = get16(bf);
	return v | get16(bf)<<16;
}

static vlong
bsize(Biobuf *bf)
{
	vlong n;
	Dir *d;

	d = dirfstat(Bfildes(bf));
	if(d == nil)
		sysfatal("bstat: %r");
	n = d->length;
	free(d);
	return n;
}

static void
unrlew(u16int *d, u16int *s)
{
	u16int n, t, v, *e;

	s++;
	e = d + Mapa;
	t = rlewtag;
	while(d < e){
		v = *s++;
		if(v == t){
			n = *s++;
			v = *s++;
			while(n-- > 0)
				*d++ = v;
		}else
			*d++ = v;
	}
}

static void
uncarmack(Biobuf *bf, u16int *u, u32int ofs)
{
	u16int v, a[Mapa], *p, *cp, *e;
	u8int n;

	Bseek(bf, ofs, 0);
	p = a;
	e = a + get16(bf) / 2;
	while(p < e){
		v = get16(bf);
		n = v & 0xff;
		switch(v >> 8){
		tag:
			*p++ = v | get8(bf);
			break;
		copy:
			while(n-- > 0)
				*p++ = *cp++;
			break;
		case 0xa7:
			if(n == 0)
				goto tag;
			cp = p - get8(bf);
			if(cp < a)
				sysfatal("uncarmack: bad offset");
			goto copy;
		case 0xa8:
			if(n == 0)
				goto tag;
			cp = a + get16(bf);
			if(cp > p)
				sysfatal("uncarmack: bad offset");
			goto copy;
		default:
			*p++ = v;
		}
	}
	unrlew(u, a);
}

static void
unhuff(Biobuf* bf, u16int hf[], uchar *u, int len)
{
	int k;
	uchar *e, b;
	u16int *h, v;

	h = hf+2*254;
	e = u+len;
	k = 1;
	b = get8(bf);
	while(u < e){
		v = h[b & k ? 1 : 0];
		k <<= 1;
		if(k & 0x100){
			b = get8(bf);
			k = 1;
		}
		if(v < 256){
			*u++ = v & 0xff;
			h = hf+2*254;
		}else
			h = hf+2*(v-256);
	}
}

static void
deplane(uchar *d, uchar *s, int n)
{
	uchar *e;

	e = s + n;
	while(s < e){
		*d++ = s[0];
		*d++ = s[n];
		*d++ = s[n*2];
		*d++ = s[n*3];
		s++;
	}
}

static void
walls(Biobuf *bf, int n, u16int *sz, u32int *of)
{
	uchar **d, **e;

	drofs = n - 8;
	wals = emalloc(n * sizeof *wals);
	e = wals + n;
	for(d=wals; d<e; d++, of++){
		n = *sz++;
		if(n == 0)
			continue;
		Bseek(bf, *of, 0);
		*d = emalloc(n);
		eread(bf, *d, n);
	}
}

static void
sprites(Biobuf *bf, int n, u16int *sz, u32int *of)
{
	uchar *hu, *u;
	Spr *s, *e;
	Sprc **c, *ap, a[65];

	sprs = emalloc(n * sizeof *sprs);
	e = sprs + n;
	for(s=sprs; s<e; s++, of++){
		n = *sz++;
		if(n == 0)
			continue;
		Bseek(bf, *of, 0);
		s->lx = get16(bf);
		s->rx = get16(bf);
		n -= 4;
		s->sp = emalloc(n);
		eread(bf, s->sp, n);
		n = s->rx - s->lx + 1;
		s->cs = emalloc(n * sizeof *s->cs);
		s->ce = s->cs + n;
		hu = s->sp;
		c = s->cs;
		while(n-- > 0){
			u = s->sp + GBIT16(hu) - 4;
			hu += 2;
			ap = a;
			ap->e = GBIT16(u) / 2, u+=2;
			while(ap->e != 0){
				ap->p = s->sp + (s16int)GBIT16(u) - 4, u+=2;
				ap->s = GBIT16(u) / 2, u+=2;
				ap->p += ap->s;
				ap++;
				ap->e = GBIT16(u) / 2, u+=2;
			}
			ap++;
			*c = emalloc((ap-a) * sizeof **c);
			memcpy(*c, a, (ap-a) * sizeof **c);
			c++;
		}
	}
}

static void
pcmpak(Biobuf *bf, int n, u16int *sz, u32int *of)
{
	u16int *csz, *cp, *ce;
	uchar *u;
	vlong sof;
	Dat *d, *e;

	sof = Bseek(bf, 0, 1);
	Bseek(bf, of[n], 0);
	n = sz[n] / 4;
	csz = emalloc(n * sizeof *csz);
	cp = csz;
	ce = csz + n;
	while(cp < ce){
		Bseek(bf, 2, 1);
		*cp++ = get16(bf);
	}
	npcm = n;
	pcms = emalloc(n * sizeof *pcms);
	Bseek(bf, sof, 0);
	for(d=pcms, e=pcms+n, cp=csz; d<e; d++){
		n = *cp++;
		if(*sz == 0){
			n = (n-1) / 4096 + 1;
			sz += n;
			of += n;
			continue;
		}
		d->p = emalloc(n * sizeof *d->p);
		d->e = d->p + n;
		u = d->p;
		while(u < d->e){
			Bseek(bf, *of++, 0);
			u += eread(bf, u, *sz++);
		}
	}
	free(csz);
}

static void
vswap(void)
{
	int n, so, po;
	u32int *ofs, *ofp;
	u16int *szs, *szp;
	Biobuf *bf;

	bf = bopen("vswap.", OREAD);
	n = get16(bf);
	so = get16(bf);
	po = get16(bf);
	ofs = emalloc(n * sizeof *ofs);
	szs = emalloc(n * sizeof *szs);
	ofp = ofs;
	while(ofp < ofs + n)
		*ofp++ = get32(bf);
	szp = szs;
	while(szp < szs + n)
		*szp++ = get16(bf);
	walls(bf, so, szs, ofs);
	sprites(bf, po - so, szs + so, ofs + so);
	pcmpak(bf, n - po - 1, szs + po, ofs + po);
	free(ofs);
	free(szs);
	Bterm(bf);
}

static void
gamemaps(void)
{
	int n;
	u32int v, *d;
	Biobuf *hed;

	hed = bopen("maphead.", OREAD);
	n = ver==WL6 ? 60 : ver==WL1 ? 10 : ver==SDM ? 2 : 21;
	rlewtag = get16(hed);
	d = mapofs = emalloc(n * sizeof *mapofs);
	mape = d + n;
	while(d < mape){
		v = get32(hed);
		if(v == 0xffffffff)
			sysfatal("sparse map %zud", d-mapofs);
		*d++ = v;
	}
	Bterm(hed);
}

static void
swap(Sfx *a, Sfx *b)
{
	Sfx c;

	memcpy(&c, a, sizeof c);
	memcpy(a, b, sizeof c);
	memcpy(b, &c, sizeof c);
}

static void
mungesfx(void)
{
	uchar *p, *e;
	Dat *pcm;

	if(ver >= SDM){
		swap(sfxs+Sscream4, sfxs+Shansdie);
		swap(sfxs+Sscream5, sfxs+Shitlerdie);
		swap(sfxs+Sscream7, sfxs+Seva);
		swap(sfxs+Sscream8, sfxs+Shans);
		swap(sfxs+Sscream6, sfxs+Smechdie);
		swap(sfxs+Sscream9, sfxs+Sschbdie);
	}
	p = pcmt[ver<SDM ? 0 : 1];
	e = p + npcm;
	for(pcm=pcms; p<e; p++, pcm++)
		if(*p != Send)
			sfxs[*p].pcm = pcm;
	sfxs[Sscream3].pcm = sfxs[ver<SDM ? Sscream2 : Sscream4].pcm;	/* why */
}

static void
al(Biobuf *dat, Biobuf *aux, int n)
{
	int m;
	u32int v;
	uchar *p;
	Sfx *s, *e;

	s = sfxs = emalloc(n * sizeof *sfxs);
	e = s + n;
	while(s < e){
		v = get32(aux);
		Bseek(dat, v, 0);
		m = get32(dat);
		s->pri = get16(dat);
		eread(dat, s->inst, sizeof s->inst);
		Bseek(dat, 6, 1);
		s->blk = (get8(dat) & 7) << 2 | 1<<5;
		p = emalloc(m);
		eread(dat, p, m);
		s->p = p;
		s->e = p + m;
		s++;
	}
}

static void
imf(Biobuf *dat, Biobuf *aux)
{
	int n;
	uchar *p;
	u32int v;
	Dat *d, *e;

	n = ver < SDM ? 27 : 24;
	d = imfs = emalloc(n * sizeof *imfs);
	e = d + n;
	while(d < e){
		v = get32(aux);
		Bseek(dat, v, 0);
		n = get16(dat);
		d->p = p = emalloc(n);
		d->e = p + n;
		eread(dat, p, n);
		d++;
	}
}

static void
audiot(void)
{
	int n;
	Biobuf *hed, *dat;

	hed = bopen("audiohed.", OREAD);
	dat = bopen("audiot.", OREAD);
	n = ver < SDM ? Send : Ssend;
	Bseek(hed, n*4, 0);
	al(dat, hed, n);
	Bseek(hed, n*4, 1);
	imf(dat, hed);
	Bterm(hed);
	Bterm(dat);

	mungesfx();
}

static int
piched(Biobuf *dat, Biobuf *aux, u16int hf[])
{
	u32int v, n;
	uchar *u, *p;
	Pic *d, *e;

	v = get24(aux);
	Bseek(dat, v, 0);
	n = get32(dat);
	p = u = emalloc(n);
	unhuff(dat, hf, u, n);
	n /= 4;
	d = pics = emalloc(n * sizeof *pics);
	e = d + n;
	while(d < e){
		d->x = GBIT16(p), p+=2;
		d++->y = GBIT16(p), p+=2;
	}
	free(u);
	return n;
}

static void
getpics(Biobuf *dat, Biobuf *aux, u16int hf[], int n)
{
	u32int v, m;
	uchar *u, *p;
	Pic *s, *e;

	s = pics;
	e = s + n;
	while(s < e){
		v = get24(aux);
		Bseek(dat, v, 0);
		m = get32(dat);
		u = emalloc(m);
		unhuff(dat, hf, u, m);
		p = emalloc(m);
		deplane(p, u, s->x*s->y/4);
		free(u);
		s++->p = p;
	}
	pict = picts[ver];
}

static void
getfnts(Biobuf *dat, Biobuf *aux, u16int hf[])
{
	int *o;
	u32int v, n;
	uchar *u, *p;
	char *w;
	Fnt *f, *e;

	f = fnts;
	e = f + 2;
	while(f < e){
		v = get24(aux);
		Bseek(dat, v, 0);
		n = get32(dat);
		p = u = emalloc(n);
		unhuff(dat, hf, u, n);
		f->h = GBIT16(p), p+=2;
		for(o=f->ofs; o < f->ofs+nelem(f->ofs); o++)
			*o = GBIT16(p) - (2+256*3), p+=2;
		for(w=f->w; w < f->w+nelem(f->w); w++)
			*w = *p++;
		n -= p-u;
		f->p = emalloc(n);
		memcpy(f->p, p, n);
		free(u);
		f++;
	}
}

static void
getexts(Biobuf *dat, Biobuf *aux, u16int hf[])
{
	int n, m;
	uchar *u, **d, **e;
	u32int v;

	n = (bsize(aux) - Bseek(aux, 0, 1)) / 3 - 1;
	d = exts = emalloc(n * sizeof *exts);
	e = d + n;
	while(d < e){
		v = get24(aux);
		Bseek(dat, v, 0);
		m = get32(dat);
		u = emalloc(m);
		unhuff(dat, hf, u, m);
		*d++ = u;
	}
	dems = exts + (ver < SOD ? 3 : 13);
	epis = dems + (ver == SDM ? 1 : 4);
}

static void
gfx(void)
{
	int n;
	u16int hf[512], *h;
	Biobuf *dat, *aux;

	aux = bopen("vgadict.", OREAD);
	for(h=hf; h<hf+nelem(hf); h++)
		*h = get16(aux);
	Bterm(aux);

	aux = bopen("vgahead.", OREAD);
	dat = bopen("vgagraph.", OREAD);
	n = piched(dat, aux, hf);
	getfnts(dat, aux, hf);
	getpics(dat, aux, hf, n);
	get24(aux);	/* ignore bullshit tile lump full of lies */
	getexts(dat, aux, hf);
	Bterm(aux);
	Bterm(dat);
}

static void
cfg(void)
{
	msense = 5;
}

static void
loadscr(void)
{
	Biobuf *bf;

	ext = ver < SDM ? "wl6" : "sod";
	bf = bopen("intro.", OREAD);
	eread(bf, pxb, Va);
	out();
	Bterm(bf);
}

static void
sttspr(void)
{
	State *s;

	for(s=stt; s<stt+GSe; s++)
		if(s->sprn != 0)
			s->spr = sprs + s->sprn;
}

static void
fixpal(void)
{
	u32int *p, *s;

	p = pals[C0] + 166;
	s = sodpal;
	while(p < pals[Cfad]){
		p[0] = *s++;
		p[1] = *s++;
		p += nelem(pals[C0]);
	}
}

u16int *
readmap(int n)
{
	u16int *u;
	u32int *m, p0, p1;
	Biobuf *dat;

	m = mapofs + n;
	if(m >= mape)
		sysfatal("readmap: invalid map number %d", n);
	dat = bopen("gamemaps.", OREAD);
	Bseek(dat, *m, 0);
	u = emalloc(Mapsz);
	p0 = get32(dat);
	p1 = get32(dat);
	Bseek(dat, 10, 1);
	if(get16(dat) != Mapdxy || get16(dat) != Mapdxy)
		sysfatal("invalid map dimensions");
	uncarmack(dat, u, p0);
	uncarmack(dat, u+Mapa, p1);
	Bterm(dat);
	return u;
}

char *
demof(char *f)
{
	char *p;
	vlong n;
	Biobuf *bf;

	bf = Bopen(f, OREAD);
	if(bf == nil)
		sysfatal("demof: %r");
	Blethal(bf, nil);
	n = bsize(bf);
	p = emalloc(n);
	eread(bf, p, n);
	Bterm(bf);
	return p;
}

void
dat(char *dir)
{
	char *e;

	rfork(RFNAMEG);
	if(bind(".", dir, MBEFORE|MCREATE) < 0 || chdir(dir) < 0)
		fprint(2, "dat: %r\n");

	e = ext;
	loadscr();
	ext = e;
	vswap();
	gamemaps();
	if(ver == SOD)
		ext = "sod";
	audiot();
	gfx();
	sttspr();
	if(ver >= SDM){
		fixpal();
		sodmap();
	}
	ext = e;
	cfg();
}
