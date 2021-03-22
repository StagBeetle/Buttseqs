#ifndef screenvars_h
#define screenvars_h
namespace scrn{
	//Maybe do this better:
	//label, x, y, labelWidth, blankWidth, modes)
	//Edit Notes:
	displayVar octave	("octave"	,	5 + (columnSize * 0),	initialHeight + (rowSize * 0),	85  ,	120,	{{modes::editNotes, modes::editSteps}} );
	displayVar viewBar	("view Bar"	,	5 + (columnSize * 0),	initialHeight + (rowSize * 1),	85  ,	120,	{{modes::editNotes, modes::editSteps}} );
	displayVar priority	("priority"	,	5 + (columnSize * 1),	initialHeight + (rowSize * 0),	100 ,	120,	{{modes::editNotes}} );
	displayVar advance	("advance"	,	5 + (columnSize * 1),	initialHeight + (rowSize * 1),	100 ,	240,	{{modes::editNotes}} );
	displayVar substep	("substep"	,	5 + (columnSize * 1),	initialHeight - (rowSize * 1),	100 ,	240,	{{modes::editNotes}} );
	//EditSteps: 	name	x		y	labelwidth	blankwidth	modes
	displayVar velocity	("velocity"	,	5 + (columnSize * 1),	initialHeight + (rowSize * 0),	85  ,	120,	{{modes::editSteps}} );
	displayVar length	("length"	,	5 + (columnSize * 1),	initialHeight + (rowSize * 1),	85  ,	140,	{{modes::editSteps}} );
	displayVar accent	("accent"	,	5 + (columnSize * 1),	initialHeight + (rowSize * 2),	85  ,	120,	{{modes::editSteps}} );
	displayVar legato	("legato"	,	5 + (columnSize * 1),	initialHeight + (rowSize * 3),	85  ,	120,	{{modes::editSteps}} );
	//All: 	name	x		y	labelwidth	blankwidth	modes
	//displayVar editPattVar	(""	,	list::dataXCoord ,	topOffset,	0 ,	100,	{{modes::editPattern}} );
	displayVar track	("track:"	,	420 ,	0,	40 ,	60,	{{false}}, scrn::td::bg, scrn::td::fg);
	displayVar tempo	("tempo:"	,	310,	0,	50  ,	110,	{{false}}, scrn::td::bg, scrn::td::fg);
	//PattUtil: 	name	x		y	labelwidth	blankwidth	modes
	displayVar pattName	("Name:"	,	5,	topOffset,	45,	135,	{{modes::patternUtils, modes::patternSwitch, modes::patternProcess, modes::copy, modes::quantise, modes::editPattern}});
	displayVar pattNum	("#:"	,	5 + 135,	topOffset,	20,	80,	{{modes::patternUtils, modes::patternSwitch, modes::patternProcess, modes::copy, modes::quantise, modes::editPattern}});
	displayVar sdStatus	(""	,	300,	topOffset,	0,	240,	{{modes::patternUtils}});
	//Other: 	name	x		y	labelwidth	blankwidth	modes
	displayVar editSteps	(""	,	320 ,	topOffset,	0,	160,	{{modes::editSteps}});
	//Process: 	name	x		y	labelwidth	blankwidth	modes
	//displayVar rangeMode	("mode"	,	5 + (columnSize * 0),	topOffset + 30,	60,	240,	{{modes::patternProcess, modes::copy}});
	//displayVar activePattNum	(""	,	5 ,	topOffset,	0,	80,	{{modes::patternProcess, modes::copy, modes::quantise}});
	//displayVar activePattName	(""	,	85 ,	topOffset,	0,	120,	{{modes::patternProcess, modes::copy, modes::quantise}});
	displayVar rangeStart	("Start:"	,	15 ,	topOffset + 20,	40,	120,	{{modes::patternProcess, modes::copy, modes::quantise}});
	displayVar rangeEnd	("End:"	,	120 ,	topOffset + 20,	40,	120,	{{modes::patternProcess, modes::copy, modes::quantise}});
	//Copy: 	name	x		y	labelwidth	blankwidth	modes	
	displayVar sourcePattNum	(""	,	240 ,	topOffset,	0,	80,	{{modes::copy, modes::quantise}});
	displayVar sourcePattName	(""	,	320 ,	topOffset,	0,	120,	{{modes::copy, modes::quantise}}); 
}
#endif