//Drawing stuff on the screen
//A layer above the screen library

//Rework list to not use pointers. It is like this for the SD card lists and the pattern lists which are dynamic. But these lists shoudl just refer to a static array which has its members changed
#ifndef functionLists_h
#define functionLists_h
#include "list.h"
namespace list{
	
typedef volatile bool* vbp;
	
std::vector<liElem*> settingsVec = 
{
	new liElemF<sF<vbp>, gF<vbp>>	("Edit priority"	, interface::settings::editPriority	, {"Note"	, "Step"	}, interface::settings::resetAfterEditToggle	),
	new liElemF<sF<voidint>, gF<intvoid>>	("Advance Steps"	, interface::settings::setAdvanceSteps	, interface::settings::getAdvanceSteps	, {0, 4096}),
	new liElemF<sF<vbp>, gF<vbp>>	("Button Note preview"	, interface::settings::notePreview	, {"No"	, "Yes"	}	),
	new liElemF<sF<vbp>, gF<vbp>>	("Note display"	, interface::settings::useSharpNotes	, {"Flat"	, "Sharp"	}	),
	new liElemF<sF<vbp>, gF<vbp>>	("Step chaser"	, interface::settings::useFancyLEDChaser	, {"Single"	, "Fancy"	}	),
	new liElemF<sF<vbp>, gF<vbp>>	("Piano roll Follows"	, interface::settings::pianoRollFollowPlay	, {"Edit Pattern"	, "Track Pattern"	}	),
	new liElemF<sF<vbp>, gF<vbp>>	("New pattern name"	, patMem::givePlaceholderNameToNewPatterns	, {"Blank"	, "pattxx"	}	),
	new liElemF<sF<vbp>, gF<vbp>>	("Duplicate pattern name"	, patMem::incrementNumberToDuplicatePatterns	, {"Clone"	, "Increment"	}	),
	new liElemF<sF<voidvoid>, gF<>>	("Edit Theme"	, []{modes::switchToMode(modes::themeEdit, true);}				),
	new liElemF<sF<vbp>, gF<vbp>>	("Edit Substeps"	, interface::settings::editSubstepsAlso	, {"Only steps"	, "Substeps"	},[]{draw::eventList();}	),
	//new liElemF<sF<vbp>, gF<vbp>>	("Midi Sync"	, Sequencing::midiSyncExternal	, {"Internal"	, "External"	}	),
	new liElemF<sF<vbp>, gF<vbp>>	("Shuffle -> Note Length"	, Sequencing::adjustNoteLengthByShuffle	, {"Not Yes"	, "Yes"	}	),
	new liElemF<sF<vbp>, gF<vbp>>	("Record With Buttons"	, interface::settings::recordOnlyFromExternal	, {"External Only"	, "Yes"	}	),
	new liElemF<sF<voidint>, gF<intvoid>>	("Screen Brightness"	, interface::settings::setScreenBrightness	, interface::settings::getScreenBrightness	, {0, 255}),
	new liElemF<sF<voidvoid>, gF<>>	("Debug Menu"	, []{modes::switchToMode(modes::debug, true);}				),
	new liElemF<sF<vbp>, gF<vbp>>	("Hold Shift Show Help"	, interface::settings::showHelp	, {"No"	, "Yes"	}	),
};

std::vector<liElem*> debugVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	("Memory Inspect"	, []{modes::switchToMode(modes::memoryInspect, true);}	),
	new liElemF<sF<voidvoid>, gF<>>	("All Notes Off"	, interface::all::sendAllNotesOff	),
};

std::vector<liElem*> memoryVec = 
{
	new liElemF<sF<vbp>, gF<vbp>>	("View"	, interface::debug::zoomed	, {"Whole"	, "Page"	}, draw::updateScreen	),
};

std::vector<liElem*> pattUtilVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	("Rename Pattern"	, interface::pattUtils::enterRenameMode	),
	new liElemF<sF<voidvoid>, gF<>>	("Save Pattern"	, []{interface::pattUtils::savePatternAsToCard();}	),
	new liElemF<sF<voidvoid>, gF<>>	("View Patterns on card"	, []{modes::switchToMode(modes::cardView, true);}	),
	new liElemF<sF<voidvoid>, gF<>>	("Connect to card"	, interface::pattUtils::connectToCard	),
};

std::vector<liElem*> pattProcVec = 
{
	new liElemF<sF<vbp>, gF<vbp>>	("Entire / Range"	,	interface::applyOnEntirePattern	, {"Range"	, "Entire"	}, interface::process::updateDisplay	),
	
	new liElemF<sF<voidvoid>, gF<>>	("Set active Pattern for process"	, []{interface::process::setActivePatternForProcess();}	),
	new liElemF<sF<voidvoid>, gF<>>	("Select Start"	, interface::process::gotoStart	),
	new liElemF<sF<voidvoid>, gF<>>	("Select End"	, interface::process::gotoEnd	),
	new liElemF<sF<voidvoid>, gF<>>	("Clear"	, interface::process::clearRange	),
	new liElemF<sF<voidvoid>, gF<>>	("Copy"	, []{modes::switchToMode(modes::copy, true);}	),
	new liElemF<sF<voidvoid>, gF<>>	("Quantise"	, []{modes::switchToMode(modes::quantise, true);}	),
};

std::vector<liElem*> copyVec = 
{
	// new liElemF<sF<voidvoid>, gF<>>	("Edit start point"	,	interface::copy::gotoStart	),
	// new liElemF<sF<voidvoid>, gF<>>	("Edit end point"	,	interface::copy::gotoEnd	),
	// new liElemF<sF<voidvoid>, gF<>>	("Paste range"	,	interface::copy::gotoPaste	),
	new liElemF<sF<voidvoid>, gF<>>	("Set current pattern as source"	,	interface::copy::selectAsSource	),
	new liElemF<sF<vbp>, gF<vbp>>	("Clear / Merge"	,	interface::copy::clearBeforeCopy	, {"Merge"	, "Clear"	}	),
	new liElemF<sF<vbp>, gF<vbp>>	("Wrap to pattern"	,	interface::copy::wrapPaste	, {"Do not wrap"	, "Wrap"	}	),
};

std::vector<liElem*> quantiseVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	(interface::quantise::quantiseLevelStrings[0]	, []{interface::quantise::setQuantiseLevel(interface::quantise::quantiseLevels[0]);} ),
	new liElemF<sF<voidvoid>, gF<>>	(interface::quantise::quantiseLevelStrings[1]	, []{interface::quantise::setQuantiseLevel(interface::quantise::quantiseLevels[1]);} ),
	new liElemF<sF<voidvoid>, gF<>>	(interface::quantise::quantiseLevelStrings[2]	, []{interface::quantise::setQuantiseLevel(interface::quantise::quantiseLevels[2]);} ),
	new liElemF<sF<voidvoid>, gF<>>	(interface::quantise::quantiseLevelStrings[3]	, []{interface::quantise::setQuantiseLevel(interface::quantise::quantiseLevels[3]);} ),
	new liElemF<sF<voidvoid>, gF<>>	(interface::quantise::quantiseLevelStrings[4]	, []{interface::quantise::setQuantiseLevel(interface::quantise::quantiseLevels[4]);} ),
	new liElemF<sF<voidvoid>, gF<>>	(interface::quantise::quantiseLevelStrings[5]	, []{interface::quantise::setQuantiseLevel(interface::quantise::quantiseLevels[5]);} ),
	new liElemF<sF<voidvoid>, gF<>>	(interface::quantise::quantiseLevelStrings[6]	, []{interface::quantise::setQuantiseLevel(interface::quantise::quantiseLevels[6]);} ),
	
	new liElemF<sF<vbp>, gF<vbp>>	("Wrap to pattern"	,	interface::quantise::wrapQuantise	, {"Do not wrap"	, "Wrap"	}	),
	
//	new liElemF<sF<voidvoid>, gF<>>	("Apply"	, interface::quantise::apply	),//Apply is automatic
};

std::vector<liElem*> performVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	("Sync Next Bar", interface::performance::syncOnBarChange	),
};

std::vector<liElem*> cardViewVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	("Load Pattern To Memory"	, []{interface::pattUtils::loadPatternToMemory();}	),
	new liElemF<sF<voidvoid>, gF<>>	("Load Pattern To Track"	, interface::pattUtils::loadPatternToActiveTrack	),
	new liElemF<sF<voidvoid>, gF<>>	("Load Pattern To Track Bank"	, interface::pattUtils::loadPatternFromCardToTrackBank	),
	new liElemF<sF<voidvoid>, gF<>>	("Delete Pattern"	, interface::pattUtils::deletePatternFromCard	),
	new liElemF<sF<voidvoid>, gF<>>	("Rename"	, interface::pattUtils::renamePatternOnCard	),
};

std::vector<liElem*> pattSwitVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	("Add Pattern to Active Track"	, interface::pattSwitch::addPatternToActiveTrack	),
	new liElemF<sF<voidvoid>, gF<>>	("Add Pattern to Track Bank"	, interface::pattSwitch::addPatternToTrackBank	),
	new liElemF<sF<voidvoid>, gF<>>	("Delete Pattern From Memory"	, interface::pattSwitch::deletePatternFromMemory	),
	new liElemF<sF<voidvoid>, gF<>>	("Clear Pattern From Active Track"	, interface::pattSwitch::clearPatternFromTrack	),
	new liElemF<sF<voidvoid>, gF<>>	("Remove a Pattern From Trackbank"	, interface::pattSwitch::removePatternFromTrackBankInitiate	), 
	new liElemF<sF<voidvoid>, gF<>>	("New Empty Pattern"	, interface::pattSwitch::newBlankPattern	),
	new liElemF<sF<voidvoid>, gF<>>	("New Duplicate Pattern"	, interface::pattSwitch::newDuplicatePattern	),
	
	new liElemF<sF<vbp>, gF<vbp>>	("View mode"	, interface::settings::jumpToActivePattern	, {"Last"	, "Jump"	},  []{list::pattSwit.draw();} ),
};

std::vector<liElem*> editStepsVec =
{
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[0],	[]{interface::editStep::setStepParameterType(0);}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[1],	[]{interface::editStep::setStepParameterType(1);}), 
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[2],	[]{interface::editStep::setStepParameterType(2);}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[3],	[]{interface::editStep::setStepParameterType(3);}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[4],	[]{interface::editStep::setStepParameterType(4);}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[5],	[]{interface::editStep::setStepParameterType(5);}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[6],	[]{interface::editStep::setStepParameterType(6);}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[7],	[]{interface::editStep::setStepParameterType(7);}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[8],	[]{interface::shiftNotes::goToShiftNotesMode();}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[9],	[]{interface::editStep::setStepParameterType(9);}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[10],	[]{interface::editStep::setStepParameterType(10);}),
	new liElemF<sF<voidvoid>, gF<>>	(interface::editStep::paramNames[11],	[]{interface::editStep::setStepParameterType(11);}),
	
	new liElemF<sF<vbp>, gF<vbp>>	("Edit"	, interface::settings::editSubstepsAlso	, {"Only steps"	, "Substeps"	},[]{draw::eventList(); interface::editStep::setListPositionToLast();}	),
	
	new liElemF<sF<voidvoid>, gF<>>	("Clear Steps"	, interface::editNotes::clearSteps, []{interface::editStep::setListPositionToLast();}	, true),
};

std::vector<liElem*> editPattVec = 
{
	new liElemF<sF<voidint>, gF<intvoid>>	("Steps per bar"	, interface::editPatt::setStepsPerBar	, []()->int	{return Sequencing::getActivePattern().getStepsPerBar();}	 ),
	new liElemF<sF<voidint>, gF<intvoid>>	("Pattern Length"	, interface::editPatt::setStepLength	, []()->int	{return Sequencing::getActivePattern().getLengthInSteps();}	 , {0, 256}),
	new liElemF<sF<voidint>, gF<intvoid>>	("Pattern Length (Bars)"	, interface::editPatt::setStepLengthInBars	, []()->int	{return Sequencing::getActivePattern().getLengthInSteps();}	 , {0, 16}),
	new liElemF<sF<voidint>, gF<intvoid>>	("Velocity offset"	, interface::editPatt::setVelocity	, []()->int	{return Sequencing::getActivePattern().getVelocity();}	 , {-127, 127}),
	new liElemF<sF<voidint>, gF<intvoid>>	("Accent offset"	, interface::editPatt::setAccentVelocity	, []()->int	{return Sequencing::getActivePattern().getAccentVelocity();}	 , {-127, 127}),
	new liElemF<sF<voidint>, gF<intvoid>>	("Transpose"	, interface::transpose::goToTransposeMode	, []()->int	{return Sequencing::getActivePattern().getTranspose();}	 ),
	new liElemF<sF<voidint>, gF<intvoid>>	("Shuffle"	, interface::editPatt::setShuffle	, []()->int	{return Sequencing::getActivePattern().getShuffle();}	 ),
	new liElemF<sF<voidvoid>, gF<charpvoid>>	("Stretch to bar"	, interface::editPatt::toggleFillToBar	, []()->const char*	{return interface::editPatt::getFillToBar();}	 ),
	new liElemF<sF<voidvoid>, gF<>>	("Track Settings:"	, [/*just to break up the menu*/]{}	),
	new liElemF<sF<voidint>, gF<charpvoid>>	("Track Mode"	, interface::editPatt::setTrackMode	, []()->const char*	{return Sequencing::trackArray[interface::editTrack].getTrackModeString();}	 ),
	new liElemF<sF<voidint>, gF<intvoid>>	("Track MIDI channel"	, interface::editPatt::setTrackChannel	, []()->int	{return Sequencing::trackArray[interface::editTrack].getMIDIchannel();}	 ),
	new liElemF<sF<voidint>, gF<charpvoid>>	("Track MIDI port"	, interface::editPatt::setTrackPort	, []()->const char*	{return Sequencing::trackArray[interface::editTrack].getMIDIportForUser();}	 ),
	new liElemF<sF<voidint>, gF<charpvoid>>	("Sound Engine"	, interface::editPatt::setSoundEngine	, []()->const char*	{return Sequencing::trackArray[interface::editTrack].getSoundEngineName();}	 ),
};

std::vector<liElem*> themeEditVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	(scrn::colourDescriptors[0], []{interface::colour::setEditColourDescriptor(0);}),
	new liElemF<sF<voidvoid>, gF<>>	(scrn::colourDescriptors[1], []{interface::colour::setEditColourDescriptor(1);}),
	new liElemF<sF<voidvoid>, gF<>>	(scrn::colourDescriptors[2], []{interface::colour::setEditColourDescriptor(2);}),
	new liElemF<sF<voidvoid>, gF<>>	(scrn::colourDescriptors[3], []{interface::colour::setEditColourDescriptor(3);}),
	new liElemF<sF<voidvoid>, gF<>>	(scrn::colourDescriptors[4], []{interface::colour::setEditColourDescriptor(4);}), 
	new liElemF<sF<voidvoid>, gF<>>	(scrn::colourDescriptors[5], []{interface::colour::setEditColourDescriptor(5);}),
};

std::vector<liElem*> arrangeVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	("Add Pattern"	, interface::arrange::addPatternToTrack	),
	new liElemF<sF<voidint>, gF<intvoid>>	("Set Length"	, interface::arrange::setAddLength	, []()->int	{return interface::arrange::getAddLength();}, {1, 64}	 ),
	new liElemF<sF<voidvoid>, gF<>>	("Active to empty"	, interface::arrange::setEditPatternToEmpty	),
	new liElemF<sF<voidvoid>, gF<>>	("Active to highlighted"	, interface::arrange::setEditPatternToHighlighted	),
	new liElemF<sF<voidvoid>, gF<>>	("Active to track"	, interface::arrange::setEditPatternToCurrentTrackPattern	),
	new liElemF<sF<voidint>, gF<intvoid>>	("Append patterns"	, interface::arrange::setAppendNewPatternsForBars	, []()->int	{return interface::arrange::getAppendNewPatternsForBars();}, {0, 64}	 ),
	new liElemF<sF<voidint>, gF<charpvoid>>	("Cursor mode"	, interface::arrange::changeCursorPosition	, []()->const char*	{return interface::arrange::getCursorPositionString();}	 ),
	new liElemF<sF<voidint>, gF<intvoid>>	("Jump To"	, interface::arrange::jumpTo	, []()->int	{return interface::arrange::getTimelinePosition();}, {0, 4095}	 ),
	new liElemF<sF<voidvoid>, gF<intvoid>>	("Set loop start"	, interface::arrange::setLoopStart	, []()->int	{return Sequencing::loopStart;} ),
	new liElemF<sF<voidvoid>, gF<intvoid>>	("Set loop end"	, interface::arrange::setLoopEnd	, []()->int	{return Sequencing::loopEnd;} ),
	new liElemF<sF<vbp>, gF<vbp>>	("Use Arrange Mode"	, interface::arrange::useArrangeMode	, {"No"	, "Yes"	} ),
};

//std::function<bool()> e = []{return true;} //Some 
std::vector<liElem*> MIDIOutputVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	(""	, interface::MIDIPortScreen::toggleSendClock	),
	new liElemF<sF<voidvoid>, gF<>>	(""	, interface::MIDIPortScreen::toggleSendStartStop	),
	new liElemF<sF<voidvoid>, gF<>>	(""	, interface::MIDIPortScreen::toggleSendSeqControl	),
	new liElemF<sF<voidvoid>, gF<>>	(""	, interface::MIDIPortScreen::toggleSendSSP	),
};

std::vector<liElem*> MIDIInputVec = 
{
	new liElemF<sF<voidvoid>, gF<>>	(""	, interface::MIDIPortScreen::toggleReceiveClock	),
	new liElemF<sF<voidvoid>, gF<>>	(""	, interface::MIDIPortScreen::toggleReceiveStartStop	),
	new liElemF<sF<voidvoid>, gF<>>	(""	, interface::MIDIPortScreen::toggleReceiveSeqControl	),
	new liElemF<sF<voidvoid>, gF<>>	(""	, interface::MIDIPortScreen::toggleReceiveSSP	),
	new liElemF<sF<voidint>, gF<>>	(""	, interface::MIDIPortScreen::setRoutingType	),
	new liElemF<sF<voidint>, gF<>>	(""	, interface::MIDIPortScreen::setRoutingDestination	),
	new liElemF<sF<voidint>, gF<>>	(""	, interface::MIDIPortScreen::setRecordType	),
	new liElemF<sF<voidint>, gF<>>	(""	, interface::MIDIPortScreen::setPreviewType	),
};

std::vector<liElem*> selectVec = 
{
	new liElemF<sF<vbp>, gF<vbp>>	("Select Substeps"	, interface::settings::editSubstepsAlso	, {"No"	, "Yes"	}	),
	new liElemF<sF<voidvoid>, gF<>>	("Mode: Set"	, interface::selection::setSelectionTypeSet	),
	new liElemF<sF<voidvoid>, gF<>>	("Mode: Add"	, interface::selection::setSelectionTypeAdd	),
	new liElemF<sF<voidvoid>, gF<>>	("Mode: Remove"	, interface::selection::setSelectionTypeRemove	),
	new liElemF<sF<voidvoid>, gF<>>	("Mode: Toggle"	, interface::selection::setSelectionTypeToggle	),
	new liElemF<sF<voidvoid>, gF<>>	("Exit Substep"	, interface::selection::exitSubstepMode	),
};

std::vector<liElem*> processVec = 
{
	new liElemF<sF<voidint>, gF<>>	("Add Process"	, interface::process::addProcessToTrack),
	new liElemF<sF<voidint>, gF<>>	("Remove Process"	, interface::process::removeProcessFromTrack),
	new liElemF<sF<voidint>, gF<>>	("Reorder Process"	, interface::process::reorderProcesses),
};

//type	name	vector	type	, y	, x	, w	, scroll	, maxlines, 	, dataXoffset	, showActive
listControllerFixed	settings	= {&settingsVec	, listType::numbered	, 0	, 0	, 480	, true	, 16	, 240	, true};
listControllerFixed	debug	= {&debugVec	, listType::numbered	, 0	, 0	, 480	, true	, 16	, 240	, true};
listControllerFixed	memory	= {&memoryVec	, listType::numbered	, 260	, 300	, 480	, false	, 16	, 100	, true};
listControllerFixed	pattUtil	= {&pattUtilVec	, listType::numbered	, 40	, 0	, 480	, false	, 8	, 240	, false};
listControllerFixed	pattProc	= {&pattProcVec	, listType::numbered	, 80	, 0	, 480	, false	, 8	, 240	, false};
listControllerFixed	perform	= {&performVec	, listType::numbered	, 0	, 0	, 480	, true	, 8	, 240	, true};
listControllerFixed	cardView	= {&cardViewVec	, listType::numbered	, 180	, 0	, 480	, true	, 8	, 240	, true};
listControllerFixed	pattSwit	= {&pattSwitVec	, listType::numbered	, 30	, 180	, 300	, true	, 8	, 240	, false};
listControllerFixed	editSteps	= {&editStepsVec	, listType::numbered	, 30	, 300	, 200	, false	, 16	, 100	, true};
listControllerFixed	editPatt	= {&editPattVec	, listType::numbered	, 30	, 0	, 480	, true	, 16	, 240	, true};
listControllerFixed	copy	= {&copyVec	, listType::numbered	, 80	, 0	, 480	, false	, 8	, 240	, false};
listControllerFixed	quantise	= {&quantiseVec	, listType::numbered	, 80	, 0	, 480	, false	, 12	, 240	, false};
listControllerFixed	themeEdit	= {&themeEditVec	, listType::numbered	, 30	, 0	, 180	, false	, 6	, 240	, true};
listControllerFixed	arrange	= {&arrangeVec	, listType::numbered	, 0	, 280	, 240	, false	, 16	, 160	, false};
listControllerFixed	MIDIOutput	= {&MIDIOutputVec	, listType::numbered	, 0	, 0	, 0	, false	, 0	, 160	, false};
listControllerFixed	MIDIInput	= {&MIDIInputVec	, listType::numbered	, 0	, 0	, 0	, false	, 0	, 160	, false};
listControllerFixed	select	= {&selectVec	, listType::numbered	, 0	, 300	, 180	, false	, 8	, 155	, false};
listControllerFixed	process	= {&processVec	, listType::numbered	, 30	, 240	, 100	, false	, 8	, 240	, false};
listControllerCard	cardPatts	= {nullptr	, listType::normal	, 0	, 0	, 480	, true	, 8	, 240	, true};
listControllerPatMem	memPatts	= {nullptr	, listType::fullkeyed	, 30	, 0	, 180	, true	, 8	, 240	, true};
		
}//End list namespace
#endif