#ifndef functionDescriptions_h
#define functionDescriptions_h

#include "for_map.h"
//#include <unordered_map>
//#include "C:\Program Files (x86)\Arduino\hardware\tools\arm\arm-none-eabi\include\c++\5.4.1\bits\unordered_map.h"

namespace functionDescriptions{
	
	const char* jumpToListItem = "ERROR JTLI";
	
	forMap::for_map<modes::mode*, const char*, 15> jumpToListItemMap = {{{
		{&modes::editSteps	, "Set active parameter"	},
		{&modes::editPattern	, "Set active parameter"	},
		{&modes::patternProcess	, "Jump to list item"	},
		{&modes::copy	, "Jump to list item"	},
		{&modes::quantise	, "Jump to list item"	},
		{&modes::debug	, "Jump to list item"	},
		{&modes::memoryInspect	, "Jump to list item"	},
		{&modes::patternUtils	, "Jump to list item"	},
		{&modes::performance	, "Jump to list item"	},
		{&modes::themeEdit	, "Set active theme colour"	},
		{&modes::arrange	, "Jump to list item"	},
		{&modes::MIDIRouting	, "Set active parameter"	},
		{&modes::selection	, "Jump to list item"	},
		{&modes::settings	, "Jump to list item"	},
	}}};
	
	//std::array<std::pair<voidint, const char*>, 6> 
	forMap::for_map<voidint, const char*, 48> funcMap = {{{
		{interface::editNotes::addNoteOnStep	, "Change the current step and add a note"	},
		{interface::editNotes::setViewBarOrEnterSubstep	, "Change the active bar and exit substep or enter a substep"	},
		{interface::editNotes::addNoteOnPitch	, "Change the current pitch and add a note"	},
		{interface::editNotes::releaseNote	, "End the MIDI note if one is playing"	},
		{interface::all::keyboardOctShift	, "Change the keyboard octave"	},
		{interface::editNotes::setEditTrack	, "Change the active track"	},
		{interface::editNotes::setSubstepStep	, "Change the viewed step for substep"	},
		{interface::editNotes::setEditStep	, "Change the step selection"	},
		{interface::editStep::setViewBarOrEditSubstep	, "Change the active bar or step for substep"	},
		{interface::editNotes::setEditPitch	, "Change the pitch selection"	},
		{interface::editStep::editStepParameter	, "Modify the active parameter"	},
		{interface::all::jumpToListItem	, jumpToListItem	},
		{list::performActionOnActiveListItem	, "Modify the active parameter"	},
		{interface::all::listMove	, "Traverse List"	},
		{interface::process::rangeSet	, "Set first and last step of range"	},
		{interface::editNotes::setViewBar	, "Set active bar"	},
		{interface::process::setActivePatternForProcessFromButtons	, "Set active pattern for process"	},
		{interface::copy::pasteTo	, "Paste pattern to step"	},
		{interface::pattUtils::doCardFunction	, "Perform the listed card function"	},
		{interface::pattSwitch::switchToPattern	, "Switch to pattern"	},
		{interface::pattSwitch::doPatternListFunction	, "Perform the listed pattern function"	},
		{interface::debug::setMemoryPage	, "Set the viewed memory page"	},
		{interface::performance::tempoChangeLarge	, "Change tempo by 1.0"	},
		{interface::performance::tempoChangeSmall	, "Change tempo by 0.01"	},
		{interface::mute::toggleMuteTrack	, "Mute tracks"	},
		{interface::mute::toggleMuteNote	, "Mute notes"	},
		{interface::pattUtils::enterPatternName	, "Edit pattern name"	},
		{interface::all::exitError	, "Return to previous mode"	},
		{interface::modals::doModalFunction	, "Select and perform option"	},
		{interface::modals::doModalNumFunction	, "Select and perform option"	},
		{interface::colour::setEditColourChannel	, "Set colour channel"	},
		{interface::colour::editColourValue	, "Edit active colour channel"	},
		{interface::arrange::setEditTrackOrChangeArrangement	, "Set edit track or change arrangement"	},
		{interface::arrange::nextTrack	, "Traverse tracks"	},
		{interface::arrange::moveTimelinePosition	, "Move timeline position"	},
		{interface::arrange::selectTrackFromBank	, "Select track from bank"	},
		{interface::MIDIPortScreen::setActivePort	, "Set active in or out port"	},
		{interface::MIDIPortScreen::setActiveParameter	, "Set active parameter"	},
		{interface::MIDIPortScreen::traverseList	, "Traverse parameter list"	},
		{interface::transpose::setTranspose	, "Set track transpose"	},
		{interface::transpose::jumpBackToPatternMode	, "Return to pattern edit mode, possibly"	},
		{interface::shiftNotes::jumpBackToStepMode	, "Return to pattern edit mode, possibly"	},
		{interface::selection::selectStepOrSubstep	, "Modify select state of step or substep"	},
		{interface::selection::enterSubstepSelect	, "Go to select substeps in step mode"	},
		{interface::selection::setViewBarOrRange	, "Set active bar or range"	},
		{interface::modeSelect::setDisplayType	, "Set the displayed mode type"	},
		{interface::selection::selectAllInBar	, "Jump to mode"	},
		{interface::process::goToProcess	, "Modify or mute process"	},
	}}};
	
	//const int lineHeight = scrn::font::getTextHeight() * 1;
	const int lineHeight = 22;
	
	void displayDescriptions(){
		int counter = 0;
		modes::getActiveMode().forEachFunction(
			[&counter](buttons::keySet::ks k, buttons::buttonEvent::be e, voidint f){
				const int pos = funcMap.find(f);
				if(pos != -1){
					int yHeight = 5 + (counter*lineHeight) + scrn::topOffset;
					scrn::writeFillRect(5, yHeight , 470, lineHeight, scrn::getThemeColour(scrn::td::bg));
					//scrn::write(5, yHeight, buttons::keySetNames[k]);
					scrn::writeIcon(5, yHeight, static_cast<scrn::iconIndex>(k), scrn::getThemeColour(scrn::td::fg));
					scrn::writeIcon(5 + 45, yHeight, static_cast<scrn::iconIndex>((int)e + scrn::eventOffset) , scrn::getThemeColour(scrn::td::fg));
					//draw::keyIcon(5, yHeight, k);
					//scrn::write(90, yHeight, buttons::buttonEventNames[static_cast<int>(e)]);
					const char* description = funcMap.getValue(pos);
					if(description == jumpToListItem){
						description = jumpToListItemMap.findAndGet(&modes::getActiveMode());
					}
					// lg(description);
					scrn::write(75, yHeight, description);
					counter ++;
				} else {
					lg("FUNC NOT FOUND");
				}
			}
		);
	}

}//end namespace
#endif

	