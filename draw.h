//Drawing stuff on the screen
//A layer above the screen library
#ifndef draw_h
#define draw_h
#include "screen.h"
#include "list.h"
#include "functionlists.h"
namespace draw{
	uint16_t screenwidth  = 480;
	uint16_t screenheight = 320;

	void startup(){
		scrn::fillScreen(scrn::getThemeColour(scrn::td::bg));
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
		scrn::write(5   , 160, "S"); delay(50);
		scrn::write(60  , 160, "E"); delay(50);
		scrn::write(115 , 160, "Q"); delay(50);
		scrn::write(170 , 160, "U"); delay(50);
		scrn::write(225 , 160, "E"); delay(50);
		scrn::write(280 , 160, "N"); delay(50);
		scrn::write(335 , 160, "C"); delay(50);
		scrn::write(390 , 160, "E"); delay(50);
		scrn::write(445 , 160, "R"); delay(50);
	}

	void tempo(){
		char tempoString[8] = {0};
		sprintf(tempoString, "%.2f", Sequencing::getTempo());
		scrn::tempo.update(tempoString);
	}

	void drawMode(){
		scrn::writeFillRect(0, 0, scrn::width, scrn::topOffset, scrn::getThemeColour(scrn::td::fg));//Draw top bar
		scrn::setTextColor(scrn::getThemeColour(scrn::td::bg));
		scrn::write(30, 0, modes::getModeString());
		//scrn::track.update(interface::editTrack);
		draw::tempo();
	}
	
	void recordIcon(){
		//static bool wasRecordingLastRun = false;
		static bool hollowToggle = true;
		bool isRecordingNow = interface::isRecording();
		scrn::writeFillRect(0, 0, 20, 20, scrn::getThemeColour(scrn::td::fg));
		if(isRecordingNow){
			const int tempo = Sequencing::getTempo();
			auto drawFunc = hollowToggle ? scrn::drawCircle : scrn::drawFillCircle;
			drawFunc(10, 10, 8, scrn::getThemeColour(scrn::td::highlight));
			hollowToggle = !hollowToggle;
			scheduled::newEvent(scheduled::lOE::recordIcon, recordIcon, static_cast<int>(60000.0 / tempo));//Draw the record icon with the beats, though unsynced.
		}
	}

	void updateScreen(){
		modes::getActiveMode().updateDisplay();
	}

	namespace pianoRollConstants{
		static const uint8_t W = 2;
		static const uint8_t B = 1;
		static const uint8_t colours[12] = {W,B,W,B,W,W,B,W,B,W,B,W};// colours of the notes ascending
		static const uint8_t noteChannelWidth = 5;
		static const uint8_t noteWidth = noteChannelWidth - 2;
		static const uint8_t miniGap = 2;
		static const uint8_t numberOfNotes = 37;
		static const uint16_t heightForPianoRoll = numberOfNotes * noteChannelWidth;
		static const uint16_t widthForPianoRoll = 384;
		static const uint16_t topOffset = 27;
		static const uint16_t sideOffset = 5;
		static const uint16_t keyThickness = 20;
		static const uint16_t bottom = topOffset + heightForPianoRoll;
		static const uint16_t numberOfBeats = 32;
		static const uint16_t numberOfSubsteps = numberOfBeats * 16;
		static const uint16_t stepWidth = 12;
		static const uint8_t offset = 1;
		static const uint16_t editStepsOffset = bottom + miniGap;
		static const uint16_t editStepsHeight = 5;
		static const uint16_t pianoRollXOffset = sideOffset + keyThickness + miniGap;
		static const uint8_t legatoSize = 2;
		static const uint8_t legatoOffset = 1;
	};

	void drawPianoRollChaser(){
		if(modes::checkActive({modes::editNotes})){
			using namespace pianoRollConstants;
			scrn::writeFillRect(//Blank - step chaser
				pianoRollXOffset,
				editStepsOffset + editStepsHeight + miniGap,
				16 * stepWidth + offset,
				editStepsHeight,
				scrn::getThemeColour(scrn::td::bg)
				);

			scrn::writeFillRect(//Blank - bar chaser
				pianoRollXOffset,
				editStepsOffset + editStepsHeight + miniGap*2 + editStepsHeight,
				16 * stepWidth + offset,
				editStepsHeight,
				scrn::getThemeColour(scrn::td::bg)
				);

			if(Sequencing::isSeqPlaying()){
				int currentBar = Sequencing::trackArray[interface::editTrack].getCurrentBar();
				scrn::writeFillRect(//bar Chaser
					pianoRollXOffset + currentBar * stepWidth + offset,
					editStepsOffset + editStepsHeight + miniGap*2 + editStepsHeight,
					stepWidth - offset,
					editStepsHeight,
					scrn::getThemeColour(scrn::td::fg)
					);

				int currentStep = Sequencing::trackArray[interface::editTrack].getCurrentStep();
				// lg(currentStep);
				scrn::writeFillRect(//step Chaser
					pianoRollXOffset + currentStep * stepWidth + offset,
					editStepsOffset + editStepsHeight + miniGap,
					stepWidth - offset,
					editStepsHeight,
					scrn::getThemeColour(scrn::td::fg)
					);
			}
		}
	}

	void drawPianoRollSteps(){
		/*
		//This is commented out because it doesn't make sense with the new step selection system, but there may be something useful in here
		if(modes::checkActive({modes::editNotes})){
			using namespace pianoRollConstants;
			for(int i = 0; i<16; i++){//Empty step squares
				int stepsPerBar = Sequencing::trackArray[interface::editTrack].getStepsPerBar();
				scrn::writeFillRect(
					pianoRollXOffset + 1 + i * stepWidth,
					bottom + 2  ,
					stepWidth - 1,
					editStepsHeight,
					i < stepsPerBar ? scrn::getThemeColour(scrn::td::fg) : scrn::getThemeColour(scrn::td::acc2)
					);
			}
			const int offset2 = offset;//I don't know what this is for? Something to do with captures?
			if(interface::inSubstep()){
				scrn::writeFillRect(
					pianoRollXOffset + 1 + interface::editStepForSubstep * stepWidth + offset2,
					bottom + 2 + offset2 ,
					stepWidth - 1 - offset2*2,
					editStepsHeight - offset2*2,
					scrn::getThemeColour(0)
				);
			} else {
				interface::forEachStep([offset2](patMem::position pos){//Actual edit steps
					scrn::writeFillRect(
						pianoRollXOffset + 1 + pos.getStep() * stepWidth + offset2,
						bottom + 2 + offset2 ,
						stepWidth - 1 - offset2*2,
						editStepsHeight - offset2*2,
						scrn::getThemeColour(0)
					);
				});
			}
		}
		*/
	}

	void drawPianoRoll(bool drawAll){
		using namespace pianoRollConstants;
		if(modes::checkActive({modes::editNotes})){
			//scrn::writeFillRect(0, i*noteWidth, widthForPianoRoll, noteWidth, scrn::theme[coloursReverse[i%12]].gSC());
			if(drawAll){
				for (int i = 0; i<numberOfNotes; i++){//note channels
					scrn::writeFillRect(pianoRollXOffset, bottom - i*noteChannelWidth - noteChannelWidth, widthForPianoRoll, noteChannelWidth, scrn::getThemeColour(colours[i%12]));
					}
				for (int i = 0; i<numberOfBeats; i++){//Minor gridlines
					scrn::writeFastVLine(i*stepWidth + pianoRollXOffset, topOffset, heightForPianoRoll, scrn::getThemeColour(scrn::td::fg));
					}
				for (int i = 0; i<numberOfBeats/4; i++){//Major gridlines
					scrn::writeFastVLine(i*stepWidth*4 + pianoRollXOffset,     topOffset, heightForPianoRoll, scrn::getThemeColour(scrn::td::fg));
					scrn::writeFastVLine(i*stepWidth*4 + pianoRollXOffset + 1, topOffset, heightForPianoRoll, scrn::getThemeColour(scrn::td::fg));
					}
			}
			for (int i = 0; i<numberOfNotes; i++){//Thin note channels to clear
				scrn::writeFillRect(pianoRollXOffset, bottom - i*noteChannelWidth - noteChannelWidth + 1, widthForPianoRoll, noteWidth, scrn::getThemeColour(colours[i%12]));
				}
			for (int i = 0; i<numberOfNotes; i++){//Keys
				scrn::writeFillRect(sideOffset	, bottom - i*noteChannelWidth - noteChannelWidth + 1, keyThickness, noteChannelWidth-1, scrn::getThemeColour(colours[i%12]));
			}
			//Add notes to piano roll:
			for (int i = 0; i < numberOfSubsteps; i++){
			// for (int i = 0; i < numberOfBeats; i++){
				int bar = interface::settings::pianoRollFollowPlay ?  Sequencing::trackArray[interface::editTrack].getCurrentBar() : interface::viewBar;
				uint8_t actualBar = (bar + (i >= (numberOfSubsteps/2))) % 16;
				uint8_t actualStep = (i / 16) % 16;
				uint8_t actualSubstep = i % 16;
				std::vector<Sequencing::note> notes = Sequencing::getActiveTrack().getNotesOnStep({actualBar, actualStep, actualSubstep}, interface::settings::displayProcessedPatternsInPianoRoll);
				// int size = notes.size();
				// if(size > 0){
					// lg(notes.size());
				// }
				for(auto n: notes){
					if(n.getPitch() >= interface::gPFK(0) && n.getPitch() < interface::gPFK(numberOfNotes)){
						int xPos = pianoRollXOffset + (i * stepWidth) / 16;
						// lg(xPos);
						int relPitch = n.getPitch() - interface::gPFK(0);
						bool isLegato = n.getLegato();
						int thisNoteWidth = !isLegato ? min((stepWidth * n.getLengthInSubsteps())/16, widthForPianoRoll - xPos + pianoRollXOffset) : legatoSize;
						int thisNoteY = bottom - relPitch*noteChannelWidth - noteChannelWidth + 1;
						scrn::td::themeDescriptor descriptor = Sequencing::trackArray[interface::editTrack].isNoteMuted(n.getPitch()) ? scrn::td::bg : scrn::td::highlight;
						scrn::colour noteColour = scrn::getThemeColour(descriptor);
						if(!isLegato){
							scrn::writeFillRect(
								xPos,
								thisNoteY,
								thisNoteWidth,
								noteWidth,
								noteColour
							);
						} else {
							scrn::writeFillRect(
								xPos,
								thisNoteY,
								noteWidth,
								noteWidth,
								noteColour
							);
							scrn::writeFillTri(
								xPos + noteWidth	, thisNoteY - 1	,
								xPos + noteWidth	, thisNoteY - 1 + noteWidth	,
								xPos + noteWidth + (noteWidth*4)	, thisNoteY - 1 + noteWidth	,
								scrn::getThemeColour(scrn::td::highlight)
								//{180, 90, 20}
							);
						}
					}
				}
			}
			if(interface::settings::editPriority == interface::editPriorityModes::noteFirst){//Active pitch indicators
				interface::forEachPitch([](int i){
					if(i >= interface::gPFK(0) && i < interface::gPFK(numberOfNotes)){
						
						int xPos = sideOffset + 1;
						int relPitch = i - interface::gPFK(0);
						scrn::writeFillRect(
							xPos,
							bottom - relPitch*noteChannelWidth - noteChannelWidth + 2,
							keyThickness - 2 ,
							noteWidth - 1,
							//scrn::getThemeColour(colours[i % 12] == W ? 0 : 3)
							scrn::getThemeColour(scrn::td::bg)
							);
					}
				});
			}
		drawPianoRollChaser();
		drawPianoRollSteps();
		}
	}
	void eventList(){//For showing the events on screen
		if(modes::checkActive({modes::editSteps , modes::shiftNotes})){
			// lg("drawList");
			scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
			if(Sequencing::getActivePattern().isValid()){
				// lg("isValid");
				const uint8_t leftMargin = 5;
				const uint8_t lineHeight = 20;
				const uint8_t charSpace = 10;
				const uint8_t headergap = 5;
				const int offsets[] = {
					leftMargin+charSpace*0, // "pos"
					leftMargin+charSpace*2, //
					leftMargin+charSpace*4, //
					leftMargin+charSpace*7, // "key"
					leftMargin+charSpace*12,// "vel"
					leftMargin+charSpace*17,// "len"
					leftMargin+charSpace*19,//
					leftMargin+charSpace*21,//
					leftMargin+charSpace*24,// "a"
					leftMargin+charSpace*26 // "l"
				};
				const int numberOfRows = 10;
				//scrn::writeFillRect(0, scrn::topOffset, offsets[9]+30, (numberOfRows+2) * lineHeight + 5, scrn::getThemeColour(scrn::td::bg));
				scrn::writeFillRect(0, scrn::topOffset, offsets[9]+30, 250, scrn::getThemeColour(scrn::td::bg));
				//Bar, step, pitch, velocity, length, ACC?, SLide?
				std::vector<Sequencing::notePos> notesToShow;
				unsigned int numberOfShownNotes = 0;
				interface::forEachStepAndPitch([&notesToShow, &numberOfShownNotes](uint8_t p, patMem::position pos){
					auto noteToShow = Sequencing::getActivePattern().getNoteOnStep(pos,{p});
					if (noteToShow.isValid()){
						numberOfShownNotes++;
						if (notesToShow.size() < numberOfRows){
							notesToShow.push_back(Sequencing::notePos(noteToShow.getNote(), pos));
						}
					}
				});
				const int size = notesToShow.size();
				scrn::write(offsets[0], scrn::topOffset, "pos");
				scrn::write(offsets[3], scrn::topOffset, "key");
				scrn::write(offsets[4], scrn::topOffset, "vel");
				scrn::write(offsets[5], scrn::topOffset, "len");
				scrn::write(offsets[8], scrn::topOffset, "a");
				scrn::write(offsets[9], scrn::topOffset, "l");

				for(int i = 0; i<size; i++){
					char pitch[4] = {0};
					utl::getNoteFromNoteNumber(pitch, notesToShow[i].n.getPitch());
					scrn::write(offsets[0], scrn::topOffset + lineHeight + headergap + i*lineHeight, notesToShow[i].p.getBar());
					scrn::write(offsets[1], scrn::topOffset + lineHeight + headergap + i*lineHeight, notesToShow[i].p.getStep());
					scrn::write(offsets[2], scrn::topOffset + lineHeight + headergap + i*lineHeight, notesToShow[i].p.getSubstep());
					scrn::write(offsets[3], scrn::topOffset + lineHeight + headergap + i*lineHeight, pitch);
					scrn::write(offsets[4], scrn::topOffset + lineHeight + headergap + i*lineHeight, notesToShow[i].n.getVelocity());
					scrn::write(offsets[5], scrn::topOffset + lineHeight + headergap + i*lineHeight, notesToShow[i].n.getLength().getBar());
					scrn::write(offsets[6], scrn::topOffset + lineHeight + headergap + i*lineHeight, notesToShow[i].n.getLength().getStep());
					scrn::write(offsets[7], scrn::topOffset + lineHeight + headergap + i*lineHeight, notesToShow[i].n.getLength().getSubstep());
					scrn::write(offsets[8], scrn::topOffset + lineHeight + headergap + i*lineHeight, notesToShow[i].n.getAccentString());
					scrn::write(offsets[9], scrn::topOffset + lineHeight + headergap + i*lineHeight, notesToShow[i].n.getLegatoString());
				}
				char notes[25] = {0};
				sprintf(notes, "%i", numberOfShownNotes);
				strcat(notes, " note");
				if(numberOfShownNotes != 1){
					strcat(notes, "s");
				}
				strcat(notes, " selected");
				scrn::write(leftMargin, scrn::topOffset + lineHeight + headergap + size*lineHeight + 5, notes);
			} else {//Pattern not valid
				scrn::write(30, 30, "no pattern on track");
			}
		}
	}

	void drawPlayingSteps(){
		const int stepHeight = 15;
		const int stepWidth = 25;
		const int sideOffset = 15 + 2;
		const int gap = 3;
		const int offset = 1;
		const int numberOfSteps = Sequencing::getActiveTrack().getStepsPerBar();

		scrn::writeFillRect(0, scrn::topOffset, scrn::width, stepHeight, scrn::getThemeColour(scrn::td::bg));
		scrn::write(0, scrn::topOffset + 2, Sequencing::trackArray[interface::editTrack].getCurrentBar());

		for(int i = 0; i< numberOfSteps; i++){
			scrn::writeFillRect(
				sideOffset + i*(stepWidth+gap),
				scrn::topOffset + 2,
				stepWidth ,
				stepHeight,
				scrn::getThemeColour(scrn::td::fg)
			);
		}

		const int currrentStep = Sequencing::trackArray[interface::editTrack].getCurrentStep();
		scrn::writeFillRect(
			sideOffset + currrentStep*(stepWidth+gap) + offset,
			scrn::topOffset + 2  + offset,
			stepWidth - 2*offset,
			stepHeight - 2*offset,
			scrn::getThemeColour(scrn::td::acc1)
		);

	}

	void updateEditNotesVars(){//Show the variables in edit notes mode
		//Draw variables:
		scrn::octave.update(interface::keyboardOctave);
		scrn::viewBar.update(interface::viewBar);
		//scrn::tempo.update(Sequencing::timingObject::getTempo());
		scrn::priority.update(interface::settings::editPriority == interface::editPriorityModes::stepFirst ? "step" : "note" );
		scrn::advance.update(interface::settings::advanceSteps ? "yes" : "no");
		if(interface::inSubstep()){
			scrn::substep.update(interface::editStepForSubstep);
		} 
		else {
			scrn::substep.update("-");
		}
	}

	void updateEditStepVars(){//Show the variables in edit notes mode
		//Draw variables:
		scrn::octave.update(interface::keyboardOctave);
		scrn::viewBar.update(interface::viewBar);

		scrn::velocity.update(interface::editVelocity);
		char length[10] = {0};
		interface::editLength.makeString(length);
		scrn::length.update(length);
		scrn::accent.update(utl::getYesOrNo(interface::editAccent));
		scrn::legato.update(utl::getYesOrNo(interface::editLegato));

		scrn::editSteps.update(interface::editStep::paramNames[static_cast<int>(interface::editStep::activeEditStepMode)]);
	}

	void drawSquareWithChar(const uint16_t x, const uint16_t y, const uint16_t side, const char character, const scrn::colour bg, const scrn::colour fg){
		scrn::writeFillRect(x, y, side, side, bg);
		char shortstr[2] = {0};
		shortstr[0] = character;
		scrn::write(x+2, y, shortstr);
		}

	void keyboardButtons(){//Show the buttons and what they do - useful for rename screen
		const int buttonSize = 19;
		scrn::setTextColor(scrn::getThemeColour(scrn::td::bg));
		int inc = 0;
		const int stepKeys = gc::keyNum::steps;
		const int keyboardKeys = gc::keyNum::notes;
		const int remainingDataKeys = gc::numberOfCharacters - stepKeys - keyboardKeys;

		int offsetX = 5;
		int offsetY = scrn::height - buttonSize - 5;
		//Step keys
		for(int i=0; i<stepKeys; i++){
			drawSquareWithChar(offsetX + 20 * i, offsetY, buttonSize, gc::chars[inc], scrn::getThemeColour(scrn::td::fg), scrn::getThemeColour(scrn::td::bg));
			inc++;
			}
		//Note keys
		uint8_t xCoords[keyboardKeys] = {0,	1,	2,	3,	4,	6,	7,	8,	9,	10,	11,	12,	14,	15,	16,	17,	18,	20,	21,	22,	23,	24,	25,	26,	28};
		uint8_t yCoords[keyboardKeys] = {0,	1,	0,	1,	0,	0,	1,	0,	1,	0,	1,	0,	0,	1,	0,	1,	0,	0,	1,	0,	1,	0,	1,	0,	0};
		for(int i=0; i<keyboardKeys; i++){
			drawSquareWithChar(offsetX + 10 * xCoords[i], offsetY - 40 - (20 * yCoords[i]), buttonSize, gc::chars[inc], scrn::getThemeColour(scrn::td::fg), scrn::getThemeColour(scrn::td::bg));
			inc++;
			}
		//Data keys
		for(int i=0; i<remainingDataKeys; i++){
			const int oX = offsetX + 20*17;
			const int oY = offsetY - 60;
			drawSquareWithChar(oX + (i%4)*20, oY + (i/4)*20, buttonSize, gc::chars[inc], scrn::getThemeColour(scrn::td::fg), scrn::getThemeColour(scrn::td::bg));
			inc++;
			}
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
	}

	void editColours(){
		const int margin = 17;
		const int boxWidth = 60;
		const int boxHeight = 96;
		const int bgEdge = 2;
		const int yPos = scrn::height - margin - boxHeight;
		scrn::writeFillRect(0, yPos - scrn::font::getTextHeight(), scrn::width, scrn::height , scrn::getThemeColour(scrn::td::bg));
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
		scrn::writeFillRect(margin - bgEdge, yPos - bgEdge, boxWidth + bgEdge*2 , boxHeight + bgEdge*2, scrn::getThemeColour(scrn::td::fg));
		for(int i=0; i<scrn::td::max; i++){
			const int xPos = margin + (margin + boxWidth) * i;
			scrn::writeFillRect(xPos, yPos, boxWidth, boxHeight, scrn::getThemeColour(i));
			scrn::write(xPos, yPos - scrn::font::getTextHeight(), scrn::colourDescriptors[i]);
		}
	}

	void editColoursVars(){
		scrn::writeFillRect(0, scrn::topOffset, scrn::width, scrn::font::getTextHeight() , scrn::getThemeColour(scrn::td::bg));
		scrn::write(5, scrn::topOffset, scrn::colourDescriptors[interface::colour::editColourDescriptor]);
		scrn::write(5 + 100, scrn::topOffset, scrn::colourChannelNames[interface::colour::editColourChannel]);
		scrn::write(5 + 150, scrn::topOffset, interface::colour::value);
	}

	namespace muteConstants{
		static const uint16_t margin	= 2;
		static const uint16_t outerMargin	= 7;
		static const uint16_t innerIndicatorMargin	= 1;
		static const uint16_t numberOfTracks	= gc::numberOfTracks;
		static const uint16_t numberOfTrackGaps	= numberOfTracks + 1;
		static const uint16_t trackWidth	= (scrn::width - outerMargin*2 - (numberOfTrackGaps * margin)) / numberOfTracks;
		static const uint16_t innerTrackSize	= trackWidth - innerIndicatorMargin * 2;
		static const uint16_t numberOfOctaves	= 4;
		static const uint16_t numberOfBlackNotes	= numberOfOctaves * 5;
		static const uint16_t numberOfWhiteNotes	= 7 * numberOfOctaves + 1;
		static const uint16_t numberOfNoteMargins	= numberOfWhiteNotes + 1;
		static const uint16_t numberOfNotes	= numberOfBlackNotes + numberOfWhiteNotes;
		static const uint16_t noteWidth	= (scrn::width - outerMargin*2 - (numberOfWhiteNotes * margin)) / numberOfWhiteNotes;
		static const uint16_t innerNoteSize	= noteWidth - innerIndicatorMargin * 2;
		static const uint16_t notesY	= 160;
		static const uint16_t tracksY	= 220;
	};

	void muteNotes(){
		if(modes::checkActive(modes::mute)){
			using namespace muteConstants;
			uint8_t xCoords[12] = {0,	1,	2,	3,	4,	6,	7,	8,	9,	10,	11,	12};
			uint8_t yCoords[12] = {0,	1,	0,	1,	0,	0,	1,	0,	1,	0,	1,	0};
			scrn::setTextColor(scrn::getThemeColour(scrn::td::fg));
			scrn::writeFillRect(0, notesY + noteWidth, scrn::width, scrn::font::getTextHeight() , scrn::getThemeColour(scrn::td::bg)); //Text blank
			for(int i = 0; i < numberOfNotes; i++){
				const int xOffset = (xCoords[i%12] + ((i/12) * 14));
				const int xPos = outerMargin + margin + xOffset * ((noteWidth + margin)/2);
				const int yPos = notesY - yCoords[i%12] * (noteWidth + margin);
				int octave = constrain(interface::keyboardOctave, 1, 8);
				int pitch = ((octave - 1) * 12) + i;
				if(pitch < 128){
					scrn::writeFillRect(xPos, yPos, noteWidth, noteWidth , scrn::getThemeColour(scrn::td::fg));
					if(i % 12 == 0 && i/12 < 4){
						char noteName[4] = {0};
						utl::getNoteFromNoteNumber(noteName, pitch);
						scrn::write(xPos, yPos + noteWidth, noteName);
					}
					if(Sequencing::trackArray[interface::editTrack].isNoteMuted(pitch)){
						scrn::writeFillRect(xPos + innerIndicatorMargin, yPos + innerIndicatorMargin, innerNoteSize, innerNoteSize , scrn::getThemeColour(scrn::td::bg));
					}
				}
			}
		}
	}

	void muteTracks(){
		if(modes::checkActive(modes::mute)){
			using namespace muteConstants;
			scrn::setTextColor(scrn::getThemeColour(scrn::td::fg));
			scrn::writeFillRect(0, tracksY - scrn::font::getTextHeight(), scrn::width, scrn::font::getTextHeight() , scrn::getThemeColour(scrn::td::bg)); //Text blank
			for(int i = 0; i < numberOfTracks; i++){
				const int xPos = outerMargin + margin + (trackWidth + margin) * i;
				scrn::writeFillRect(xPos, tracksY, trackWidth, trackWidth , scrn::getThemeColour(scrn::td::fg));
				scrn::write(xPos, tracksY - scrn::font::getTextHeight(), i);
				if(i == interface::editTrack){
					scrn::writeFillRect(outerMargin + margin, tracksY + trackWidth + margin*2, scrn::width - (outerMargin + margin)*2, trackWidth/2 , scrn::getThemeColour(scrn::td::bg));
					scrn::writeFillRect(xPos, tracksY + trackWidth + margin*2, trackWidth, trackWidth/2 , scrn::getThemeColour(scrn::td::highlight));
				}
				if(Sequencing::trackArray[i].getMuted()){
					scrn::writeFillRect(xPos + innerIndicatorMargin, tracksY + innerIndicatorMargin, innerTrackSize, innerTrackSize , scrn::getThemeColour(scrn::td::bg));
				}
			}
		}
	}

	void mute(){
		muteNotes();
		muteTracks();
	}

	void memoryInspect(){
		uint16_t start = 0;
		uint16_t limit = 15;
		scrn::blankScreen();
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
		for(uint16_t i = start; i<start+limit; i++){
			int yHeight = 5 + (i * 20);
			blocks::block_t testBlock = {i};
			scrn::write(5	, yHeight, i);
			scrn::write(30	, yHeight, testBlock.getTypeName());
			// lg(testBlock.getTypeName());
		}
	}
	
	namespace buttonIconConstants{
		const int iconWidth = 40;
		const int iconHeight = 20;
		const int keyWidth = 8;
		const int spacing = 10;
	}
	/*
	void keyIcon (const int x, const int y, const buttons::keySet::ks key){
		using namespace buttonIconConstants;

		scrn::writeFillRect(x, y, iconWidth, iconHeight, scrn::getThemeColour(scrn::td::fg));
		switch(key){
			case buttons::keySet::step:
				for(int i = 0; i <4 ;i ++){
					scrn::writeFillRect(x+1 + (i*spacing), y+6, keyWidth, keyWidth, scrn::getThemeColour(scrn::td::bg));
				}
				break;
			case buttons::keySet::note:
				for(int i = 0; i <2 ;i ++){
					scrn::writeFillRect(x+spacing + (i*spacing), y+1, keyWidth, keyWidth, scrn::getThemeColour(scrn::td::bg));
				}
				for(int i = 0; i <3 ;i ++){
					scrn::writeFillRect(x+5 + (i*spacing), y+11, keyWidth, keyWidth, scrn::getThemeColour(scrn::td::bg));
				}
				break;
			case buttons::keySet::vertical:
				for(int i = 0; i <2 ;i ++){
					scrn::writeFillRect(x+18, y+2 + (i*spacing), keyWidth, keyWidth, scrn::getThemeColour(scrn::td::bg));
				}
				break;
			case buttons::keySet::horizontal:
				for(int i = 0; i <2 ;i ++){
					scrn::writeFillRect(x+spacing + (i*spacing), y+6, keyWidth, keyWidth, scrn::getThemeColour(scrn::td::bg));
				}
				break;
			case buttons::keySet::data:
				for(int i = 0; i <2 ;i ++){
					scrn::writeFillRect(x+spacing + (i*spacing), y+1, keyWidth, keyWidth, scrn::getThemeColour(scrn::td::bg));
				}
				for(int i = 0; i <2 ;i ++){
					scrn::writeFillRect(x+spacing + (i*spacing), y+11, keyWidth, keyWidth, scrn::getThemeColour(scrn::td::bg));
				}
				break;
			default:
				break;
		}
	}
	
	void actionIcon (const int x, const int y, const buttons::buttonEvent::be){
		using namespace buttonIconConstants;
		scrn::writeFillRect(x, y, iconWidth, iconHeight, scrn::getThemeColour(scrn::td::fg));
		if(key == buttons::buttonEvent::press || key == buttons::buttonEvent::release){
			draw::writeLine	
		}
		if(key == buttons::buttonEvent::press || key == buttons::buttonEvent::hold){
			
		}
		*/

	void patternName(patMem::pattern_t patt){//Show the current pattern name
		if(patt.isValid()){
			char name[9] = {0};
			patt.getName(name);
			scrn::pattName.update(name);
		} else {
			scrn::pattName.update("<nopatt>");
		}
	}

	void patternNumber(patMem::pattern_t patt){
		if(patt.isValid()){
			char pattNum[9] = {0};
			Sequencing::numberToPatNum(patt.getAddress(), pattNum);
			scrn::pattNum.update(pattNum);
		} else {
			scrn::pattNum.update("<nopatt>");
		}
	}

	void patternNameAndNumber(patMem::pattern_t patt){
		patternName(patt);
		patternNumber(patt);
	}

	namespace arrangeConstants{
		const int numberOfBars	= interface::arrange::numberOfBars;
		const int barWidth	= 8;
		const int trackHeight	= 14;
		//const int trackGutter	= 1;
		const int topMargin	= 10;
		const int sideMargin	= 25;
		const int gridWidth	= numberOfBars * barWidth;
		const int gridHeight	= trackHeight * 16;
		const int chaserWidth	= 2;
	}
	
	void arrangeTracks(){//This can be a lot more efficient than drawing the squares individually, probably
		using namespace arrangeConstants;
		//scrn::writeFillRect(0, scrn::topOffset, 260, 200, scrn::getThemeColour(scrn::td::bg)); //Background blank
		scrn::writeFillRect(sideMargin, scrn::topOffset+topMargin, gridWidth, gridHeight, scrn::getThemeColour(scrn::td::acc2)); //Background blank
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
		
		//Draw Vertical lines and bar numbers
		scrn::writeFillRect(0, scrn::topOffset, sideMargin + gridWidth, topMargin, scrn::getThemeColour(scrn::td::bg)); //Background blank for bar numbers
		scrn::writeFillRect(0, scrn::topOffset, sideMargin, topMargin + gridHeight, scrn::getThemeColour(scrn::td::bg)); //Background blank for track numbers
		for(int j = 0; j< numberOfBars; j++){
			int x = sideMargin + j*barWidth;
			int barNum = interface::arrange::firstTimelinePosition + j;
			
			scrn::writeFastVLine(x, scrn::topOffset + topMargin, gridHeight, scrn::getThemeColour(scrn::td::acc1));
			if(barNum % 4 == 0){
				scrn::writeFastVLine(x+1, scrn::topOffset + topMargin, gridHeight, scrn::getThemeColour(scrn::td::acc1));
				scrn::write(sideMargin + (j * barWidth), scrn::topOffset, barNum);//Bar number
			}
		}
		
		//Blankers:
		for(int i = 0; i< gc::numberOfTracks; i++){
			int y = scrn::topOffset + topMargin + i * trackHeight;
			scrn::writeFillRect(sideMargin, y + chaserWidth , gridWidth, trackHeight - chaserWidth*2, scrn::getThemeColour(scrn::td::acc2)); // blank
			scrn::writeFillRect(sideMargin, y , gridWidth, chaserWidth, scrn::getThemeColour(scrn::td::bg)); // gutter
		}
		
		for(int i = 0; i< gc::numberOfTracks; i++){
			int y = scrn::topOffset + topMargin + i * trackHeight;

			//scrn::writeFillRect(sideMargin, y+1, gridWidth, trackHeight-1, scrn::getThemeColour(scrn::td::bg)); //Track lanes
			patMem::pattern_t lastPatt = {};
			int firstOfThisPattern = 0;
			bool drawBox = false;
			
			scrn::write(2, y, i); //Track Number
			
			for(int j = 0; j< numberOfBars; j++){
				drawBox = false;
				//int x = sideMargin + j*barWidth;
				patMem::pattern_t thisPatt = arrangement::arrangeChannels[interface::arrange::activeArrangement][i].getCurrentPattern(interface::arrange::firstTimelinePosition + j);
				//When it finds a new pattern, draw the last pattern
				if(lastPatt != thisPatt){
					drawBox = true;
				}
				if((drawBox && lastPatt.isValid()) || (j == numberOfBars-1 && lastPatt.isValid())){
					char pattName[gc::maxPatternNameLength+1] = {0};
					lastPatt.getName(pattName);
					//scrn::writeFillRect(x, y, gridWidth-(j*barWidth), trackHeight, scrn::getThemeColour(scrn::td::acc2)); //
					scrn::writeFillRect(sideMargin + firstOfThisPattern*barWidth, y+chaserWidth, (j-firstOfThisPattern)*barWidth, trackHeight-chaserWidth*2, scrn::getThemeColour(scrn::td::bg)); //
					scrn::write(sideMargin + firstOfThisPattern*barWidth, y, pattName);
					// lgc("DrewBox: ");
					// lgc(firstOfThisPattern);
					// lgc(" to ");
					// lg(j-firstOfThisPattern);
					//scrn::writeFillRect(sideMargin + firstOfThisPattern*barWidth, y+2, (31-firstOfThisPattern)*barWidth, trackHeight-2, scrn::getThemeColour(scrn::td::acc1)); //
				}
				if(drawBox){
					lastPatt = thisPatt;
					firstOfThisPattern = j;
				}
			}//End bar loop
		}//End track loop
		
		//Draw selected box:
		if(interface::arrange::editTimelinePosition < interface::arrange::firstTimelinePosition + numberOfBars && interface::arrange::editTimelinePosition >= interface::arrange::firstTimelinePosition){
			int selectedPos = interface::arrange::editTimelinePosition - interface::arrange::firstTimelinePosition;
			scrn::writeDrawRect(
				sideMargin + selectedPos*barWidth,
				scrn::topOffset + topMargin + interface::editTrack * trackHeight,
				barWidth * 1,
				trackHeight,
				scrn::getThemeColour(scrn::td::highlight)
			);
		}
		
		//Draw loop points:
		int startPos	= constrain(Sequencing::loopStart - interface::arrange::firstTimelinePosition	, 0, numberOfBars);
		int endPos	= constrain(Sequencing::loopEnd - interface::arrange::firstTimelinePosition	, 0, numberOfBars);
		
		//Blank:
		scrn::writeFillRect(
			sideMargin,
			scrn::topOffset + topMargin + gridHeight + 2,
			numberOfBars * barWidth,
			3,
			scrn::getThemeColour(scrn::td::bg)
		);	
			
		if(startPos != endPos){
			//Actual:
			scrn::writeFillRect(
				sideMargin + startPos*barWidth,
				scrn::topOffset + topMargin + gridHeight + 2,
				(endPos-startPos) * barWidth,
				3,
				scrn::getThemeColour(scrn::td::highlight)
			);
		}
		
		arrangeChaser();
	}

	void arrangeChaser(){
		int static lastPositionInGrid = 0;
		if(!modes::checkActive(modes::arrange)){return;}
		using namespace arrangeConstants;
		//Blankers:
		double position =	Sequencing::playingBar + Sequencing::lengthThroughBar;
		position = position - interface::arrange::firstTimelinePosition;
		int positionInGrid = 0;
		if(position >= 0 && position < numberOfBars){
			positionInGrid = position * barWidth;
		}
		if(lastPositionInGrid != positionInGrid){
			for(int i = 0; i< gc::numberOfTracks + 1; i++){
				int y = scrn::topOffset + topMargin + i * trackHeight;
				scrn::writeFillRect(sideMargin + lastPositionInGrid, y - chaserWidth, chaserWidth, chaserWidth, scrn::getThemeColour(scrn::td::bg));
				scrn::writeFillRect(sideMargin + positionInGrid, y - chaserWidth, chaserWidth, chaserWidth, scrn::getThemeColour(scrn::td::highlight));
			}
		}
		lastPositionInGrid = positionInGrid;
		//interface::arrange::timelineTick;
	}

	void drawPatternNameAndNumber(const char* label, patMem::pattern_t patt, const int x, const int y){
		scrn::writeFillRect(x, y, 360, scrn::font::getTextHeight(), scrn::getThemeColour(scrn::td::bg));
		//scrn::writeFillRect(0, y, 480, scrn::font::getTextHeight(), scrn::getThemeColour(scrn::td::highlight));
		// lg(label);
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
		scrn::write(x, y, label);
		if(patt.isValid()){
			char pattName[9] = {0};
			patt.getName(pattName);
			char pattNum[9] = {0};
			Sequencing::numberToPatNum(patt.getAddress(), pattNum);
			scrn::write(x+100, y, pattNum);
			scrn::write(x+100+80, y, pattName);
		} else {
			scrn::write(x+100, y, "<nopatt>");
		}
	}

	void arrangeDetails(){
		drawPatternNameAndNumber("active"	, interface::arrange::editPattern	, 5	, 265);
		drawPatternNameAndNumber("selected"	, interface::arrange::highlightedPattern	, 5	, 285);
		scrn::writeFillRect(5, 305, 100, scrn::font::getTextHeight(), scrn::getThemeColour(scrn::td::bg));
		scrn::write(5, 305, interface::arrange::editTimelinePosition);
	}

	namespace routingConstants{
		const int sideMargin = 10;
		const int firstColumn = 80;
		const int underlineOffset = 15;
		const int underlineEndGap = 10;
		const int lineHeight = 18;
	}

	int getRow(const int i){
		using namespace routingConstants;
		return scrn::topOffset + 0 + (lineHeight * i);
	}

	void routingMatrix(){
		//Setup:
		using namespace routingConstants;
		scrn::writeFillRect(0, scrn::topOffset, scrn::width, scrn::height, scrn::getThemeColour(scrn::td::bg));
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));

		//Inputs:
		const int inColumnWidth = (scrn::width - firstColumn - (2 * sideMargin) ) / 4;
		//Side election indicator:
		if(interface::MIDIPortScreen::isInputSelected){
			int pos = list::MIDIInput.getPosition();
			scrn::writeFillRect(0, getRow(pos+2), sideMargin - 2, lineHeight, scrn::getThemeColour(scrn::td::highlight));
		}
		scrn::write(sideMargin, getRow(1), "Inputs");
		scrn::write(sideMargin, getRow(2), "Clock");
		scrn::write(sideMargin, getRow(3), "Strt/Stp");
		scrn::write(sideMargin, getRow(4), "Seq");
		scrn::write(sideMargin, getRow(5), "SSP");
		scrn::write(sideMargin, getRow(6), "Route");
		scrn::write(sideMargin, getRow(7), "Route");
		scrn::write(sideMargin, getRow(8), "RecType");
		scrn::write(sideMargin, getRow(9), "Preview");
		//Loop:
		for(int i = 0; i <gc::numberOfMIDIIns; i++){
			MIDIports::midiInput& m = MIDIports::inputs[i];
			int column = sideMargin + firstColumn + i * inColumnWidth;
			scrn::write(column, getRow(1), m.name);
			//Input election indicator:
			if(interface::MIDIPortScreen::selectedInputs.test(i) && interface::MIDIPortScreen::isInputSelected){
				scrn::writeFillRect(column, getRow(1)+underlineOffset, inColumnWidth - underlineEndGap, 3, scrn::getThemeColour(scrn::td::highlight));
			}
			//Rows:
			scrn::write(column, getRow(2), m.receiveClock	?	"rcv" : "-" );
			scrn::write(column, getRow(3), m.receiveStartStop	? "rcv" : "-" );
			scrn::write(column, getRow(4), m.receiveSeqControl	? "rcv" : "-" );
			scrn::write(column, getRow(5), m.receiveSSP	? "rcv" : "-" );
			scrn::write(column, getRow(6), m.getRoutingTypeString() );
			const int routDestRow = 7;
			switch(m.getRoutingType()){
				case MIDIports::routingType::track:
					scrn::write(column, getRow(routDestRow),	m.getRoutingDestination());
					break;
				case MIDIports::routingType::port:
					scrn::write(column, getRow(routDestRow),	MIDIports::getOutputName(m.getRoutingDestination()));
					break;
				default:
					scrn::write(column, getRow(routDestRow),	"-");
					break;
			}
			scrn::write(column, getRow(8), m.getRecordTypeString() );
			scrn::write(column, getRow(9), m.getPreviewTypeString() );
		}
		//Outputs:
		//Side Selection indicator:
		if(!interface::MIDIPortScreen::isInputSelected){
			int pos = list::MIDIOutput.getPosition();
			scrn::writeFillRect(0, getRow(pos+12), sideMargin - 2, lineHeight, scrn::getThemeColour(scrn::td::highlight));
		}
		const int outColumnWidth = (scrn::width - firstColumn - (2 * sideMargin) ) / 6;
		scrn::write(sideMargin, getRow(11), "Outputs");
		scrn::write(sideMargin, getRow(12), "Clock");
		scrn::write(sideMargin, getRow(13), "Strt/Stp");
		scrn::write(sideMargin, getRow(14), "Seq");
		scrn::write(sideMargin, getRow(15), "SSP");
		//For loop:
		for(int i = 0; i <gc::numberOfMIDIOuts; i++){
			MIDIports::midiOutput& m = MIDIports::outputs[i];
			int column = sideMargin + firstColumn + i * outColumnWidth;
			scrn::write(column, getRow(11), m.name);
			if(interface::MIDIPortScreen::selectedOutputs.test(i) && !interface::MIDIPortScreen::isInputSelected){
				scrn::writeFillRect(column, getRow(11)+underlineOffset, outColumnWidth - underlineEndGap, 3, scrn::getThemeColour(scrn::td::highlight));
			}

			scrn::write(column, getRow(12),	m.sendClock	?	"snd" : "-" );
			scrn::write(column, getRow(13),	m.sendStartStop	? "snd" : "-" );
			scrn::write(column, getRow(14),	m.sendSeqControl	? "snd" : "-" );
			scrn::write(column, getRow(15),	m.sendSSP	? "snd" : "-" );
		}
	}

	void viewBar(){
		if(modes::checkActive({modes::patternProcess, modes::copy})){
			int xOffset = 10;
			int yOffset = 70;
			scrn::writeFillRect(xOffset, yOffset, 120, 20, scrn::getThemeColour(scrn::td::bg));
			scrn::write(xOffset	,	yOffset	,	"viewBar"	);
			scrn::write(xOffset + 70	,	yOffset	,	interface::viewBar	);
		}
	}
	
	void transpose(){
		using namespace muteConstants;
		const int tracksY = 60;
		const int height = 80;
		const int boxHeight = 25;
		scrn::writeFillRect(0, tracksY, scrn::width, height, scrn::getThemeColour(scrn::td::bg));
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
		for(int i=0; i<16; i++){
			int transpose = Sequencing::getPattern(i).getTranspose();
			const int xPos = outerMargin + margin + (trackWidth + margin) * i;
			scrn::colour colour = (i == interface::editTrack) ? scrn::getThemeColour(scrn::td::highlight) : scrn::getThemeColour(scrn::td::acc1);
			scrn::writeFillRect(xPos, tracksY, boxHeight, boxHeight, colour);
			scrn::write(xPos + 5, tracksY + 10, transpose);
			scrn::write(xPos + 5, tracksY + 10 + boxHeight, i);	
		}
	}
	
	// enum class blockType{
		// none	= 0,
		// pattern	= 1,
		// contents	= 2,
		// arrange	= 3,
		// tempoTime	= 4,
		// mute	= 5,
		// patternSet	= 6,
		// process	= 7,
		// other	= 8,
		// max	= 9,
	// };
	
	scrn::colour blockColours[]{
		{255	, 255	, 255	}, //none	
		{255	, 0	, 255	}, //pattern	
		{0	, 255	, 255	}, //contents	
		{255	, 0	, 255	}, //arrange	
		{0	, 255	, 255	}, //tempoTime	
		{0	, 255	, 255	}, //mute	
		{0	, 255	, 255	}, //patternSet	
		{0	, 255	, 255	}, //process	
		{0	, 255	, 255	}, //other	
		{0	, 255	, 255	}, //max	
	};
	
	namespace memoryUsageConstants{
		const int blockHeight = 245;
		const int blockWidth = blockHeight;
		const int squareSize = 4;
		const int squareSizeSquared = squareSize*squareSize;
		const int zoomedBlockWidth = blockWidth/squareSize;
		const int yMargin = (scrn::height - scrn::topOffset - blockHeight) / 2 + scrn::topOffset;
		const int xMargin = yMargin;
	}
	
	void memoryUsage(){
		using namespace memoryUsageConstants;
		uint16_t selected = interface::debug::selectedBlock;
		//int lastMicros = micros();
		scrn::writeFillRect(xMargin, yMargin, blockWidth + 10, blockHeight + 10, scrn::getThemeColour(scrn::td::bg));
		if(interface::debug::zoomed){
			for(int i=0; i<blocks::numberOfBlocks/squareSizeSquared;i++){
				//delayMicroseconds(1);
				int newI = i + interface::debug::memoryPage * (blocks::numberOfBlocks / squareSizeSquared);
				//Sequencing::doSequencingIfTime(); 
				scrn::colour blockColour = blockColours[static_cast<int>(blocks::block_t(newI).getType())];
				int xPos = (i%zoomedBlockWidth) * squareSize;
				int yPos = (i/zoomedBlockWidth) * squareSize;
				scrn::writeFillRect(xMargin + xPos, yMargin + yPos, squareSize, squareSize, blockColour);
				//Selected position:
				if(
					(selected > interface::debug::memoryPage * (blocks::numberOfBlocks / squareSizeSquared)) && 
					(selected < (interface::debug::memoryPage+1) * (blocks::numberOfBlocks / squareSizeSquared))
				){
					int xSelectedPos = (selected%zoomedBlockWidth) * squareSize;
					int ySelectedPos = (selected/zoomedBlockWidth) * squareSize;
					scrn::writeDrawRect(xMargin + xSelectedPos-1, yMargin + ySelectedPos-1, squareSize+2, squareSize+2, scrn::getThemeColour(scrn::td::highlight));
				}
				
			}
		} else {
			for(int i=0; i<blocks::numberOfBlocks;i++){
				scrn::colour blockColour = blockColours[static_cast<int>(blocks::block_t(i).getType())];
				int xPos = i%blockWidth;
				int yPos = i/blockWidth;
				scrn::writePixel(xMargin + xPos, yMargin + yPos, blockColour);
				//Selected position:
				int xSelectedPos = selected%zoomedBlockWidth;
				int ySelectedPos = selected/zoomedBlockWidth;
				scrn::writeDrawRect(xMargin + xSelectedPos-1, yMargin + ySelectedPos-1, 3, 3, scrn::getThemeColour(scrn::td::highlight));
			}
		}
	}
	
	namespace selectionConstants{
		const int xMargin = 15;
		const int yMargin = scrn::topOffset + 10;
		const int substepSize = 3;
		const int substepGap = 1;
		const int stepGap = 2;
		const int substepSizeWithMargin = substepSize + substepGap;
		const int stepSize = substepSize * 4 + substepGap * 3;
		const int stepSizeWithMargin = stepSize + stepGap;
		const int wholeWidth = (stepSize) * 16 + (stepGap*15);
	}
	
	std::array<int, 2> getStepCoords(const int number){
		using namespace selectionConstants;
		const int x = xMargin + (number % 16) * stepSizeWithMargin;
		const int y = yMargin + (number / 16) * stepSizeWithMargin;
		return {x, y};
	}
	
	void selectionBorders(){
		using namespace selectionConstants;
		static int lastBar;
		static int lastStep = -1;
		const int bar = interface::viewBar;
		const int step = interface::editStepForSubstep;
		
		const int barYPos = getStepCoords(bar*16)[1];
		const int lastbarYPos = getStepCoords(lastBar*16)[1];
		lg("selectionBorders");
		// lgc(" lastBar:");
		// lg(lastBar);
		lgc(" lastStep:");
		lg(lastStep);
		// lgc(" Bar:");
		// lg(bar);
		lgc(" Step:");
		lg(step);
		
		//Clear Last:
		scrn::writeDrawRect(xMargin-1, lastbarYPos-1, wholeWidth+2, stepSize+2, scrn::getThemeColour(scrn::td::bg));
		if(lastStep >= 0){
			lg(" drawLastStep");
			std::array<int, 2> stepCoords = getStepCoords(lastBar*16 + lastStep);
			scrn::writeDrawRect(stepCoords[0]-1, stepCoords[1]-1, stepSize+2, stepSize+2, scrn::getThemeColour(scrn::td::bg));
		}
		
		//Draw new:
		scrn::writeDrawRect(xMargin-1, barYPos-1, wholeWidth+2, stepSize+2, scrn::getThemeColour(scrn::td::highlight));
		if(step >= 0){
			lg(" drawStep");
			std::array<int, 2> stepCoords = getStepCoords(bar*16 + step);
			scrn::writeDrawRect(stepCoords[0]-1, stepCoords[1]-1, stepSize+2, stepSize+2, scrn::getThemeColour(scrn::td::highlight));
		}
		
		lastBar = bar;
		lastStep = step;
	}
	
	void selectionVars(){
		using namespace selectionConstants;
		scrn::writeFillRect(300, 180, 180, 140, scrn::getThemeColour(scrn::td::bg));
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
		
		scrn::write(300, 160, "select mode");
		scrn::write(400, 160, interface::selection::getSelTypeName());
		
		scrn::write(300, 200, "viewBar");
		scrn::write(400, 200, interface::viewBar);
		
		if(interface::editStepForSubstep == -1){
			scrn::write(300, 220, "Selecting Steps");
		}
		else{
			scrn::write(300, 220, "In Step");
			scrn::write(400, 220, interface::editStepForSubstep);
		}
		selectionBorders();
	}
	
	void selection(){
		using namespace selectionConstants;
		for(int i=0; i<gc::numberOfSubsteps / 16; i++){
			std::array<int, 2> coords = getStepCoords(i);
			const int x = coords[0];
			const int y = coords[1];
			for(int j=0; j<16; j++){
				const int stepNumber = i * 16 + j;
				scrn::colour blockColour = interface::stepSelection.test(stepNumber) ? scrn::getThemeColour(scrn::td::highlight) : scrn::getThemeColour(scrn::td::acc1);
				const int xs = x + (j % 4) * substepSizeWithMargin;
				const int ys = y + (j / 4) * substepSizeWithMargin;
				scrn::writeFillRect(xs, ys, substepSize, substepSize, blockColour);
			}
		}
		selectionVars();
	}
	
	void modeList(){
		const int tabWidth = 120;
		const int tabHeight = 40;

		const int tab = static_cast<int>(interface::modeSelect::typeToDisplay);
		for(int i=0; i<4; i++){
			scrn::colour tabColour = tab==i ? scrn::getThemeColour(scrn::td::highlight) : scrn::getThemeColour(scrn::td::acc1);
			scrn::writeFillRect(i*tabWidth, scrn::topOffset, tabWidth, tabHeight, tabColour);
			scrn::write(i*tabWidth + 10, scrn::topOffset, modes::modeTypeNames[i]);
		}
		scrn::writeFillRect(0, scrn::topOffset + tabHeight + 20, 480, tabHeight*4, scrn::getThemeColour(scrn::td::bg));
		for(int i=0; i<16; i++){
			const int ID = interface::modeSelect::modesDisplaying[i];
			if(ID == -1){return;}
			const int x = 0	+ (i % 4) * tabWidth;
			const int y = scrn::topOffset + tabHeight + 20	+ (i / 4) * tabHeight;
			scrn::writeFillRect(x, y, tabWidth, tabHeight, scrn::getThemeColour(scrn::td::acc2));
			scrn::write(x + 10, y, modes::getModeByID(ID).getName());
		}
	}
	
	namespace processConstants{
		const int lineHeight = 30;
		const int boxWidth = 480 / 2 / 4;
		const int xOffset = 240;
		const int yOffset = 180;
	}
	
	void process(){
		using namespace processConstants;
		volatile Sequencing::track& t = Sequencing::getActiveTrack();
		for(int i = 0; i<t.getNumProcesses(); i++){
			const int type = t.getProcess(i).getProcessID();
			scrn::write(10, i*lineHeight, process::processes[type].getName());
			scrn::write(100, i*lineHeight, "params here");
		}
	}
	
	void processSelection(){
		using namespace processConstants;
		int counter = 0;
		
		scrn::setTextColor(scrn::getThemeColour(scrn::td::text));
		//static_assert(sizeof(process::processes) == 4 * sizeof(process::processType), "wrongsize");

		//Sort out scroll
		for(auto& proc : process::processes){
			const int x = (counter % 4) * boxWidth + xOffset;
			const int y = (counter / 4) * lineHeight + yOffset;
			// lgc(x);
			// lgc(" ");
			// lg(y);
			scrn::write(x, y, proc.getName());
			counter++;
		}
	}
	
	void buttons(const int x, const int y, const buttons::keySet::ks setToHighlight, const int scale){//Draw a digital representation of the buttons on the screen
		using namespace scrn;
		using namespace buttons;
		
		writeFillRect(x	, y+8*scale	,  16*scale	, 1*scale	,setToHighlight == keySet::step	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x	, y+6*scale	,  15*scale	, 1*scale	,setToHighlight == keySet::note	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x+1*scale	, y+5*scale	,  2*scale	, 1*scale	,setToHighlight == keySet::note	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x+4*scale	, y+5*scale	,  3*scale	, 1*scale	,setToHighlight == keySet::note	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x+8*scale	, y+5*scale	,  2*scale	, 1*scale	,setToHighlight == keySet::note	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x+11*scale	, y+5*scale	,  3*scale	, 1*scale	,setToHighlight == keySet::note	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x+17*scale	, y+5*scale	,  1*scale	, 3*scale	,setToHighlight == keySet::vertical	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x+16*scale	, y+6*scale	,  2*scale	, 1*scale	,setToHighlight == keySet::horizontal	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x+7*scale	, y	,  4*scale	, 4*scale	,setToHighlight == keySet::data	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x+12*scale	, y	,  4*scale	, 4*scale	,setToHighlight == keySet::mode	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
		writeFillRect(x+17*scale	, y	,  1*scale	, 4*scale	,setToHighlight == keySet::extra	? getThemeColour(td::highlight) : getThemeColour(td::acc1));
	}

	//void drawEdit
}
#endif