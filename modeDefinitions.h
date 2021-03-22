#ifndef modes_definitions_h
#define modes_definitions_h
namespace buttons{
	std::function<void()> allButtons[gc::keyNum::extras][buttons::buttonEvent::max] = {
		{interface::all::startButton	, nullptr	, nullptr},
		{interface::all::stopButton	, nullptr	, nullptr},
		{interface::record::toggleRecMode	, nullptr	, nullptr}, //[]{while(true){Sequencing::doSequencingIfTime();}}},
		{interface::all::setShiftOn	, interface::all::setShiftOff	, interface::all::displayHelp},
	};
}

namespace modes{

typedef buttons::keySet::ks	k;
typedef buttons::buttonEvent::be	b;
//using i = interface;
	
mode editNotes = {"Edit Notes", modeType::creation, true, false, true, true, false, nullptr, {},
	[]{
		draw::updateEditNotesVars();
		draw::drawPianoRoll(true);
	},
	{
		{k::step,	LEDfeedback::showStepSelectionOrStepsContainingNote	},
		{k::note,	LEDfeedback::showNoteSelectionOrNotesOnStep	},
		{k::horizontal,	LEDfeedback::showKeyboardOctave	},
		{k::data,	LEDfeedback::showActiveTrack	},
	},
	{
		{k::step,	b::release,	interface::editNotes::addNoteOnStep	},
		{k::step,	b::hold,	interface::editNotes::setSubstepOrEnterBar	},
		{k::note,	b::press,	interface::editNotes::addNoteOnPitch	},
		{k::note,	b::release,	interface::editNotes::releaseNote	},
		{k::horizontal,	b::press,	interface::all::keyboardOctShift	},
		{k::data,	b::press,	interface::editNotes::setEditTrack	},
	},
	{
		nullptr,
		interface::performance::tempoChangeLargeEnc,
		interface::performance::tempoChangeSmallEnc,
		interface::all::changeVolume,
	}
};

// mode substep = {"Substep" , modeType::creation, true, false, true, true, false, nullptr, {},
	// []{
		// //DRaw some substep stuff
		// draw::drawPianoRoll(true);
	// },
	// {
		// {k::step,	LEDfeedback::showStepSelectionOrStepsContainingNote	},
		// {k::note,	LEDfeedback::showNoteSelectionOrNotesOnStep	},
		// {k::horizontal,	LEDfeedback::showKeyboardOctave	},
		// {k::data,	LEDfeedback::showStepsContainingNotes	},
	// }, 
	// {
		// {k::step,	b::release,	interface::editNotes::addNoteOnStep	},
		// {k::step,	b::hold,	interface::editNotes::setSubstepOrEnterBar	},
		// {k::note,	b::press,	interface::editNotes::addNoteOnPitch	},
		// {k::note,	b::release,	interface::editNotes::releaseNote	},
		// {k::horizontal,	b::press,	interface::all::keyboardOctShift	},
		// {k::data,	b::press,	interface::editNotes::setSubstepStep	},
	// }
// };

mode editSteps = {"Edit Steps", modeType::creation, true, false, true, true, false, &list::editSteps, {},
	[]{
		draw::updateEditStepVars();
		draw::eventList();
	},
	{
		{k::step,	LEDfeedback::showStepSelection	},
		{k::note,	LEDfeedback::showNoteSelection	},
		{k::horizontal,	LEDfeedback::showKeyboardOctave	},
	},
	{
		{k::step,	b::release,	interface::editNotes::setEditStep	},
		{k::step,	b::hold,	interface::editStep::setViewBarOrEditSubstep	},
		{k::note,	b::press,	interface::editNotes::setEditPitch	},
		{k::data,	b::release,	interface::editStep::editStepParameter	},
		{k::data,	b::hold,	interface::all::jumpToListItem	},
		{k::horizontal,	b::press,	interface::all::keyboardOctShift	},
	},
	{}
};

// mode substepEdit = {"Edit Substep", modeType::creation, true, false, true, true, false, &list::editSteps, {},
	// []{
		// draw::updateEditStepVars();
		// draw::eventList();
	// },
	// {
		// {k::step,	LEDfeedback::showStepSelection	},
		// {k::note,	LEDfeedback::showNoteSelection	},
		// {k::horizontal,	LEDfeedback::showKeyboardOctave	},
	// },
	// {
		// {k::step,	b::release,	interface::editNotes::setEditStep	},
		// {k::step,	b::hold,	interface::editStep::setViewBarOrEditSubstep	},
		// {k::note,	b::press,	interface::editNotes::setEditPitch	},
		// {k::data,	b::release,	interface::editStep::editStepParameter	},
		// {k::data,	b::hold,	interface::all::jumpToListItem	},
		// {k::horizontal,	b::press,	interface::all::keyboardOctShift	},
	// }
// };

mode editPattern = {"Edit Pattern", modeType::creation, true, false, true, true, false, &list::editPatt, {},
	[]{
		draw::patternNameAndNumber();
	},
	{
		{k::step,	LEDfeedback::showActiveTrack	},
	}, 
	{
		{k::step,	b::press,	interface::editNotes::setEditTrack	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::data,	b::press,	list::performActionOnActiveListItem	},
		{k::vertical,	b::press,	interface::all::listMove	},
	},
	{}
};
mode patternProcess = {"Pattern Process" , modeType::process, false, false, true, true, false, &list::pattProc, {},
	[]{
		interface::process::setActivePatternForProcess();
		interface::process::updateDisplay();
		draw::viewBar();
	}, 
	{
		//{k::step,	LEDfeedback::showStepRange	},
		{k::data,	LEDfeedback::showActiveTrack	},
	}, 
	{
		{k::step,	b::release,	interface::process::rangeSet	},
		{k::step,	b::hold,	interface::editNotes::setViewBar	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::vertical,	b::press,	interface::all::listMove	},
		{k::data,	b::press,	interface::process::setActivePatternForProcessFromButtons	},
	},
	{}
};
mode copy = {"Copy Steps", modeType::process, true, false, true, true, false, &list::copy, {},
	[]{
		interface::copy::updateDisplay();
		draw::viewBar();
	},
	{
		//{k::step,	LEDfeedback::showStepRange	},
	}, 
	{
		{k::step,	b::release,	interface::copy::pasteTo	},
		{k::step,	b::hold,	interface::editNotes::setViewBar	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::data,	b::press,	interface::process::setActivePatternForProcessFromButtons	},
	},
	{}
};

mode quantise = {"Quantise", modeType::process, true, false, true, true, false, &list::quantise, {},
	[]{
		interface::process::setActivePatternForProcess(Sequencing::getActivePattern());
	},
	{
	}, 
	{
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::data,	b::press,	interface::process::setActivePatternForProcessFromButtons	},
	},
	{}
};

mode cardView = {"Card Browser", modeType::admin, false, false, true, true, false, &list::cardFiles, {},
	[]{
		list::cardView.draw();
	},
	{}, 
	{
		{k::note,	b::press,	interface::pattUtils::doCardFunction	},
		{k::vertical,	b::press,	interface::all::listMove	},
	},
	{}
};

mode patternSwitch = {"Pattern Switch" , modeType::live, true, false, true, true, false, &list::memPatts, {},
	[]{
		if(interface::settings::jumpToActivePattern){interface::pattSwitch::switchToActive();}
		list::pattSwit.draw();
		draw::patternNameAndNumber();
	}, 
	{
		{k::step,	LEDfeedback::showActiveTrack	},
		{k::data,	LEDfeedback::showAvailableTrackBank	},
	}, 
	{
		{k::step,	b::press,	interface::editNotes::setEditTrack	},
		{k::data,	b::press,	interface::pattSwitch::switchToPattern	},
		{k::note,	b::press,	interface::pattSwitch::doPatternListFunction	},
		{k::vertical,	b::press,	interface::all::listMove	},
	},
	{}
};
mode settings = {"Settings" , modeType::admin, false, false, true, true, false, &list::settings,  {},
	[]{
	}, 
	{
	}, 
	{		
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::vertical,	b::press,	interface::all::listMove	},
		{k::data,	b::press,	list::performActionOnActiveListItem	},
	},
	{}
};
mode debug = {"Debug" , modeType::admin, false, false, true, true, false, &list::debug,  {},
	[]{
	}, 
	{
	}, 
	{		
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::vertical,	b::press,	interface::all::listMove	},
		{k::data,	b::press,	list::performActionOnActiveListItem	},
	},
	{}
};
mode memoryInspect = {"Memory Inspect" , modeType::admin, false, false, true, true, false, &list::memory,  {},
	[]{
		draw::memoryUsage();
	}, 
	{
	}, 
	{
		{k::step,	b::press,	interface::debug::setMemoryPage	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
	},
	{}
};
mode patternUtils = {"Pattern Utilities" , modeType::admin, false, false, true, true, false, &list::pattUtil,  {},
	[]{
		
			draw::patternNameAndNumber();
			scrn::sdStatus.update(card::getConnectedString());
	}, 
	{
		{k::data,	LEDfeedback::showActiveTrack	},
	}, 
	{
		{k::vertical,	b::press,	interface::all::listMove	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::data,	b::press,	interface::editNotes::setEditTrack	},
	},
	{}
};
mode performance = {"Performance" , modeType::live, false, false, true, true, false, &list::perform,  {},
	[]{
	}, 
	{
		{k::data,	LEDfeedback::showActiveTrack	},
	}, 
	{
		{k::vertical,	b::press,	interface::performance::tempoChangeLarge	},
		{k::horizontal,	b::press,	interface::performance::tempoChangeSmall	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::data,	b::press,	interface::editNotes::setEditTrack	},
	},
	{}
};
mode mute = {"Mute" , modeType::live, false, false, true, true, false, nullptr,  {},
	[]{
		draw::mute();
	}, 
	{
		{k::data,	LEDfeedback::showActiveTrack	},
		{k::step,	LEDfeedback::showTrackMutes	},
		{k::note,	LEDfeedback::showNoteMutes	},
		{k::vertical,	LEDfeedback::showKeyboardOctave	},
	}, 
	{
		{k::data,	b::press,	interface::editNotes::setEditTrack	},
		{k::step,	b::press,	interface::mute::toggleMuteTrack	},
		{k::note,	b::press,	interface::mute::toggleMuteNote	},
		{k::vertical,	b::press,	interface::all::keyboardOctShift	},
	},
	{}
};

// mode rename = {"Rename" , modeType::admin, false, false, true, true, true, nullptr,  {},
	// []{
		// draw::keyboardButtons();
		// draw::patternName();
	// },
	// {
	// }, 
	// {
		// {k::step,	b::press,	interface::pattUtils::enterPatternName	},
	// },
	// {}
// };

// mode error = {"Error" , modeType::unlisted,  false, true, false, false, true, nullptr, {},
	// []{
	// },
	// {
	// }, 
	// {
		// {k::step,	b::press,	interface::all::exitError	},
	// },
	// {}
// };
// mode modal = {"Modal" , modeType::unlisted, true, true, false, false, false, nullptr, {},
	// []{
	// },
	// {
	// }, 
	// {
		// {k::data,	b::press,	interface::modals::doModalFunction	},
		// {k::note,	b::press,	interface::modals::doModalFunction	},
		// {k::step,	b::press,	interface::modals::doModalFunction	},
	// },
	// {}
// };
// mode modalNum = {"ModalNum" , modeType::unlisted, true, true, false, false, false, nullptr, {},
	// []{
	// },
	// {
	// }, 
	// {
		// {k::data,	b::press,	interface::modals::doModalNumFunction	},
		// {k::note,	b::press,	interface::modals::doModalNumFunction	},
		// {k::step,	b::press,	interface::modals::doModalNumFunction	},
	// },
	// {}
// };
mode themeEdit = {"Theme Edit" , modeType::admin, false, false, true, true, false, &list::themeEdit, {}, 
	[]{
		interface::colour::updateValueFromTheme();
		draw::editColours();
		draw::editColoursVars();
		// halt(); //for testing
	}, 
	{
		//{k::step,	LEDfeedback::showColourInBinary	},
	}, 
	{
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::step,	b::press,	interface::colour::setEditColourChannel	},
		{k::data,	b::press,	interface::colour::editColourValue	},
		{k::vertical,	b::press,	interface::all::listMove	},
	},
	{}
};
mode arrange = {"Arrange" , modeType::live,  false, false, true, true, false, &list::arrange, {k::note, k::data}, 
	[]{
		draw::arrangeTracks();
		draw::arrangeDetails();
	}, 
	{
		{k::step,	LEDfeedback::showActiveTrack	},
		{k::data,	LEDfeedback::showAvailableTrackBank	},
	}, 
	{
		{k::step,	b::press,	interface::arrange::setEditTrackOrChangeArrangement	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::vertical,	b::press,	interface::arrange::nextTrack	},
		{k::horizontal,	b::press,	interface::arrange::moveTimelinePosition	},
		{k::data,	b::press,	interface::arrange::selectTrackFromBank	},
	},
	{}
};

mode MIDIRouting = {"MIDI Routing" , modeType::admin, false, false, true, true, false, &list::MIDIInput, {}, 
	[]{
		draw::routingMatrix();
	}, 
	{

	}, 
	{
		{k::step,	b::press,	interface::MIDIPortScreen::setActivePort	},
		{k::note,	b::press,	interface::MIDIPortScreen::setActiveParameter	},
		{k::vertical,	b::press,	interface::MIDIPortScreen::traverseList	},
		{k::data,	b::press,	list::performActionOnActiveListItem	},
	},
	{}
};
mode transpose = {"Transpose" , modeType::live, true, false, true, true, false, nullptr, {}, 
	[]{
		interface::transpose::snapBackToEditPatternMode = false;
		draw::transpose();
	}, 
	{
		{k::note,	LEDfeedback::showTranspose	},
		{k::vertical,	LEDfeedback::showKeyboardOctave	},
		{k::step,	LEDfeedback::showActiveTrack	},
	},
	{
		{k::note,	b::press,	interface::transpose::setTranspose	},
		{k::vertical,	b::press,	interface::all::keyboardOctShift	},
		{k::step,	b::press,	interface::editNotes::setEditTrack	},
		{k::data,	b::release,	interface::transpose::jumpBackToPatternMode	},
	},
	{}
};
mode shiftNotes = {"Shift Notes" , modeType::creation, true, false, true, true, false, nullptr, {}, 
	[]{
		interface::shiftNotes::snapBackToEditStepMode = false;
		draw::eventList();
		scrn::octave.update(interface::keyboardOctave);
	}, 
	{
		{k::note,	LEDfeedback::showNotesOnStep	},
		{k::vertical,	LEDfeedback::showKeyboardOctave	},
	},
	{
		{k::note,	b::press,	interface::shiftNotes::editPitch	},
		{k::vertical,	b::press,	interface::all::keyboardOctShift	},
		{k::data,	b::release,	interface::shiftNotes::jumpBackToStepMode	},
	},
	{}
};
mode selection = {"Selection" , modeType::process, true, false, true, true, false, &list::select, {}, 
	[]{
		draw::selection();
	}, 
	{

	},
	{
		{k::step,	b::release,	interface::selection::selectStepOrSubstep	},
		{k::step,	b::hold,	interface::selection::enterSubstepSelect	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::data,	b::release,	interface::selection::setViewBarOrRange	},
		{k::data,	b::hold,	interface::selection::selectAllInBar	},
	},
	{}
};
mode modeSelect = {"Mode Select" , modeType::admin, true, false, true, true, false, nullptr, {}, 
	[]{
		draw::modeList();
	}, 
	{

	},
	{
		{k::step,	b::press,	interface::modeSelect::setDisplayType	},
		{k::data,	b::release,	interface::modeSelect::goToMode	},
	},
	{}
};
mode process = {"Process" , modeType::process, false, false, true, true, false, &list::process, {k::note, k::data}, 
	[]{
		draw::process();
		draw::processSelection();
	}, 
	{
		{k::step,	LEDfeedback::showActiveTrack},
	},
	{
		{k::step,	b::press,	interface::editNotes::setEditTrack	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::data,	b::press,	interface::process::goToProcess	},
	},
	{}
};
mode editDataEvent = {"Data Events (CC)" , modeType::creation, true, false, true, true, false, &list::dataEvent, {}, 
	[]{
		draw::dataEventTimeLine();
	},
	{
		
	}, {
		{k::step,	b::release,	interface::editNotes::setEditStep	},
		{k::step,	b::hold,	interface::editNotes::setSubstepOrEnterBar	},
		{k::note,	b::press,	interface::all::jumpToListItem	},
		{k::data,	b::press,	interface::dataEdit::editParameterOrData	},
	}, 
	{}
};
// mode processSelection = {"Process Selection" , modeType::unlisted, false, true, true, true, false, nullptr, {}, //replace this just with a sort of dialog box
	// []{
		
	// }, 
	// {
		
	// },
	// {
		// {k::data,	b::press,	interface::process::goToProcess	},
	// }
// };
//Work on this later
// mode LFO = {"LFO" , modeType::admin, true, false, true, true, false, nullptr, {}, 
	// []{
		// draw::modeList();
	// }, 
	// {

	// },
	// {
		// {k::step,	b::press,	interface::LFO::setDenominator	},
		// {k::data,	b::press,	interface::LFO::setNumerator	},
	// }
// };

mode chain	= {"chain"	, modeType::live, true, false, true, true, false, nullptr, {}, []{}, {}, {}, {}};
mode sentinel	= {"sentinel"	, modeType::unlisted, true, false, true, true, false, nullptr, {}, []{}, {}, {}, {}};
}//End namespace
#endif
/*
	mode(const char* c_name, 
	c_enableChase, c_doNotReturn, c_clearScreen, c_labelled, c_allButtonsOverride,
	list::listController* c_activeList, const std::function<void()> c_screenDisplayFunc, const std::vector<buttons::keySet::ks> heldFuncSetter
	const std::vector<LEDParam> LEDs, const std::vector<buttonParam> buttons) :
	//Encoder functions
*/