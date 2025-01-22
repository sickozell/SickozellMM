#define TRIG_MODE 1
#define CV_MODE 0
#define GODOWN 0
#define GORANDOM 1
#define GOUP 2

#include "plugin.hpp"

struct MultiRouterL : Module {
	
	int mode = TRIG_MODE;
	float trigValue = 0.f;
	float prevTrigValue = 0.f;

	int currAddr = 0;
	int prevAddr = 0;

	int direction = GODOWN;

	float xFadeKnob = 0.f;
	float xFadeCoeff;

	int fadingIn = -1;
	bool fadingOut[8] = {false, false, false, false, false, false, false, false};
	float xFadeValue[8] = {1, 1, 1, 1, 1, 1, 1, 1};

	int currOutput = 0;
	int prevOutput = 0;

	int rstIn = 1;
	float rstTrig = 0.f;
	float prevRstTrig = 0.f;

	int maxOutputs = 7;

	//int chanL;
	//int chanR;

	//float outL[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//float outR[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float outL;
	float outR;

	bool firstRun = true;

	bool revAdv = false;
	bool cycle = true;
	bool initStart = false;

	unsigned int sampleRate = APP->engine->getSampleRate();

	enum ParamId {
		MODE_SWITCH,
		OUTS_PARAM,
		DIR_SWITCH,
		XFD_PARAM,
		RST_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRG_CV_INPUT,
		RST_INPUT,
		IN_LEFT_INPUT,
		IN_RIGHT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_LEFT_OUTPUT, 8),
		ENUMS(OUT_RIGHT_OUTPUT, 8),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(OUT_LIGHT, 8),
		LIGHTS_LEN
	};

	MultiRouterL() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Inputs", {"Cv", "Trig"});

		configInput(TRG_CV_INPUT, "Trg/Cv");

		configSwitch(DIR_SWITCH, 0.f, 2.f, 0.f, "Inputs", {"Down", "Random", "Up"});

		configParam(XFD_PARAM, 0.f,1.f, 0.f, "Crossfade", "ms", 10000.f, 1.f);

		configParam(RST_PARAM, 1.f,8.f, 1.f, "Reset Input");
		paramQuantities[RST_PARAM]->snapEnabled = true;

		configInput(RST_INPUT, "Reset");

		configParam(OUTS_PARAM, 1.f,8.f, 8.f, "Outputs selector");
		paramQuantities[OUTS_PARAM]->snapEnabled = true;		

		configInput(IN_LEFT_INPUT, "Left");
		configInput(IN_RIGHT_INPUT, "Right");

		configOutput(OUT_LEFT_OUTPUT, "Out1 L");
		configOutput(OUT_RIGHT_OUTPUT, "Out1 R");
		configOutput(OUT_LEFT_OUTPUT+1, "Out2 L");
		configOutput(OUT_RIGHT_OUTPUT+1, "Out2 R");
		configOutput(OUT_LEFT_OUTPUT+2, "Out3 L");
		configOutput(OUT_RIGHT_OUTPUT+2, "Out3 R");
		configOutput(OUT_LEFT_OUTPUT+3, "Out4 L");
		configOutput(OUT_RIGHT_OUTPUT+3, "Out4 R");
		configOutput(OUT_LEFT_OUTPUT+4, "Out5 L");
		configOutput(OUT_RIGHT_OUTPUT+4, "Out5 R");
		configOutput(OUT_LEFT_OUTPUT+5, "Out6 L");
		configOutput(OUT_RIGHT_OUTPUT+5, "Out6 R");
		configOutput(OUT_LEFT_OUTPUT+6, "Out7 L");
		configOutput(OUT_RIGHT_OUTPUT+6, "Out7 R");
		configOutput(OUT_LEFT_OUTPUT+7, "Out8 L");
		configOutput(OUT_RIGHT_OUTPUT+7, "Out8 R");

	}

	void onReset(const ResetEvent &e) override {
		//initStart = false;
		
		currOutput= 0;
		xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, params[XFD_PARAM].getValue()) / 1000));
		fadingIn = currOutput;

		for (int i = 0; i < 8; i++) {
			lights[OUT_LIGHT+i].setBrightness(0.f);
			fadingOut[i] = true;
		}
		Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));
		json_object_set_new(rootJ, "revAdv", json_boolean(revAdv));
		json_object_set_new(rootJ, "cycle", json_boolean(cycle));
		json_object_set_new(rootJ, "currOutput", json_integer(currOutput));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "initStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		json_t* revAdvJ = json_object_get(rootJ, "revAdv");
		if (revAdvJ)
			revAdv = json_boolean_value(revAdvJ);

		json_t* cycleJ = json_object_get(rootJ, "cycle");
		if (cycleJ)
			cycle = json_boolean_value(cycleJ);

		if (!initStart) {
			json_t* currOutputJ = json_object_get(rootJ, "currOutput");
			if (currOutputJ) {
				currOutput = json_integer_value(currOutputJ);
				if (currOutput >= 0 && currOutput < 8) {
					xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, params[XFD_PARAM].getValue()) / 1000));
					xFadeValue[currOutput] = 0;
					fadingIn = currOutput;
				} else {
					currOutput = params[RST_PARAM].getValue()-1;
					xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, params[XFD_PARAM].getValue()) / 1000));
					xFadeValue[currOutput] = 0;
					fadingIn = currOutput;
				}
			} else {
				currOutput = params[RST_PARAM].getValue()-1;
				xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, params[XFD_PARAM].getValue()) / 1000));
				xFadeValue[currOutput] = 0;
				fadingIn = currOutput;
			}
		} else {
			currOutput = params[RST_PARAM].getValue()-1;
			xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, params[XFD_PARAM].getValue()) / 1000));
			xFadeValue[currOutput] = 0;
			fadingIn = currOutput;
		}

	}

	void process(const ProcessArgs& args) override {
		
		maxOutputs = params[OUTS_PARAM].getValue();
		
		mode = params[MODE_SWITCH].getValue();

		direction = params[DIR_SWITCH].getValue();

		rstTrig = inputs[RST_INPUT].getVoltage();
		if (rstTrig > 1.f && prevRstTrig <= 1.f) {
			prevOutput = currOutput;
			lights[OUT_LIGHT+prevOutput].setBrightness(0.f);

			if (revAdv && mode == TRIG_MODE) {
				switch (direction) {
					case GOUP:
						if (currOutput > maxOutputs - 2) { 
							if (cycle)
								currOutput = 0;
						} else
							currOutput++;
					break;

					case GODOWN:
						if (currOutput < 1) {
							if (cycle)
								currOutput = maxOutputs - 1;
						} else
							currOutput--;
					break;

					case GORANDOM:
						currOutput = random::uniform() * maxOutputs;
						if (currOutput > maxOutputs)
							currOutput = maxOutputs - 1;
					break;
				}
				xFadeKnob = params[XFD_PARAM].getValue();

				if (xFadeKnob == 0) {
					xFadeValue[currOutput] = 1;
				} else {
					xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, xFadeKnob) / 1000));
					fadingIn = currOutput;
					fadingOut[prevOutput] = true;
				}

			} else {
				currOutput = params[RST_PARAM].getValue() - 1;

				xFadeKnob = params[XFD_PARAM].getValue();

				if (xFadeKnob == 0) {
					xFadeValue[currOutput] = 1;
				} else {
					xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, xFadeKnob) / 1000));
					fadingIn = currOutput;
					fadingOut[prevOutput] = true;
				}
			}
		}
		prevRstTrig = rstTrig;

		switch (mode) {
			case TRIG_MODE:
				trigValue = inputs[TRG_CV_INPUT].getVoltage();
				if (trigValue > 1.f && prevTrigValue <= 1.f) {
					prevOutput = currOutput;
					lights[OUT_LIGHT+prevOutput].setBrightness(0.f);
					switch (direction) {
						case GODOWN:
							if (currOutput > maxOutputs - 2) { 
								if (cycle)
									currOutput = 0;
							} else
								currOutput++;
						break;

						case GOUP:
							if (currOutput < 1) {
								if (cycle)
									currOutput = maxOutputs - 1;
							} else
								currOutput--;
						break;

						case GORANDOM:
							currOutput = random::uniform() * maxOutputs;
							if (currOutput > maxOutputs)
								currOutput = maxOutputs - 1;
						break;
					}

					xFadeKnob = params[XFD_PARAM].getValue();

					if (xFadeKnob == 0) {
						xFadeValue[currOutput] = 1;
					} else {
						xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, xFadeKnob) / 1000));
						fadingIn = currOutput;
						fadingOut[prevOutput] = true;
					}
				}
				prevTrigValue = trigValue;

			break;

			case CV_MODE:
				trigValue = inputs[TRG_CV_INPUT].getVoltage();
				if (trigValue > 10.f)
					trigValue = 10.f;
				else if (trigValue < 0.f)
					trigValue = 0.f;

				if (trigValue != prevTrigValue) {
					currAddr = int(trigValue / 10 * maxOutputs);
					if (currAddr >= maxOutputs)
						currAddr = maxOutputs-1;
					if (currAddr != prevAddr) {
						prevOutput = currOutput;
						lights[OUT_LIGHT+prevOutput].setBrightness(0.f);
						currOutput = currAddr;
						
						prevAddr = currAddr;

						xFadeKnob = params[XFD_PARAM].getValue();

						if (xFadeKnob != 0) {
							xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, xFadeKnob) / 1000));
							fadingIn = currOutput;
							fadingOut[prevOutput] = true;
						}

					}
				}
				prevTrigValue = trigValue;
			break;
		}

		/*
		chanL = inputs[IN_LEFT_INPUT].getChannels();
		if (inputs[IN_RIGHT_INPUT].isConnected())
			chanR = inputs[IN_RIGHT_INPUT].getChannels();
		else
			chanR = chanL;
		*/

		for (int out = 0; out < 8; out++) {

			if (outputs[OUT_LEFT_OUTPUT+out].isConnected() || outputs[OUT_RIGHT_OUTPUT+out].isConnected()) {

				if (out == currOutput) {

					if (fadingIn == out) {
						xFadeValue[out] += xFadeCoeff;
						if (xFadeValue[out] > 1) {
							xFadeValue[out] = 1;
							fadingIn = -1;
						}
					}

					//for (int c = 0; c < chanL; c++) {
						outL = inputs[IN_LEFT_INPUT].getVoltage();

						if (outL > 10.f)
							outL = 10.f;
						else if (outL < -10.f)
							outL = -10.f;

						outputs[OUT_LEFT_OUTPUT+out].setVoltage(outL * xFadeValue[out]);
					//}

					if (inputs[IN_RIGHT_INPUT].isConnected()) {
						//chanR = inputs[IN_RIGHT_INPUT].getChannels();
						//for (int c = 0; c < chanR; c++) {

							outR = inputs[IN_RIGHT_INPUT].getVoltage();

							if (outR > 10.f)
								outR = 10.f;
							else if (outR < -10.f)
								outR = -10.f;

							outputs[OUT_RIGHT_OUTPUT+out].setVoltage(outR * xFadeValue[out]);
						//}
					} else {
						//chanR = chanL;
						//for (int c = 0; c < chanR; c++) {
							outputs[OUT_RIGHT_OUTPUT+out].setVoltage(outL * xFadeValue[out]);
						//}
					}

				} else {
					
					if (fadingOut[out]) {

						//for (int c = 0; c < chanL; c++) {
							outL = inputs[IN_LEFT_INPUT].getVoltage();

							if (outL > 10.f)
								outL = 10.f;
							else if (outL < -10.f)
								outL = -10.f;

							outputs[OUT_LEFT_OUTPUT+out].setVoltage(outL * xFadeValue[out]);
						//}

						if (inputs[IN_RIGHT_INPUT].isConnected()) {
							//chanR = inputs[IN_RIGHT_INPUT].getChannels();
							//for (int c = 0; c < chanR; c++) {

								outR = inputs[IN_RIGHT_INPUT].getVoltage();

								if (outR > 10.f)
									outR = 10.f;
								else if (outR < -10.f)
									outR = -10.f;

								outputs[OUT_RIGHT_OUTPUT+out].setVoltage(outR * xFadeValue[out]);
							//}
						} else {
							//chanR = chanL;
							//for (int c = 0; c < chanR; c++) {
								outputs[OUT_RIGHT_OUTPUT+out].setVoltage(outL * xFadeValue[out]);
							//}
						}


						xFadeValue[out] -= xFadeCoeff;
						if (xFadeValue[out] < 0) {
							xFadeValue[out] = 0;
							fadingOut[out] = false;
						}

					} else {
						//outputs[OUT_LEFT_OUTPUT+out].clearVoltages();
						//outputs[OUT_RIGHT_OUTPUT+out].clearVoltages();
						outputs[OUT_LEFT_OUTPUT+out].setVoltage(0.f);
						outputs[OUT_RIGHT_OUTPUT+out].setVoltage(0.f);
					}
				}

				//outputs[OUT_LEFT_OUTPUT+out].setChannels(chanL);
				//outputs[OUT_RIGHT_OUTPUT+out].setChannels(chanR);
			}
		}
		lights[OUT_LIGHT+currOutput].setBrightness(1.f);

	}		
};

struct MultiRouterLWidget : ModuleWidget {
	MultiRouterLWidget(MultiRouterL* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/MultiRouterL.svg")));

		/*addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		*/
		const float xLeft = 6.8;
		const float yCvSw = 19.5;
		const float yCvSwIn = 35;

		const float xDir = 5.8;
		const float yDir = 59.8;

		const float yXfd = 85.6;
		const float yRstKn = 105.7;
		const float yRstIn = 116;

		const float yIn = 18.8;

		const float xOuts = 28.5;
		//const float yIns = 15.6;
		const float yOuts = 35.1;

		const float xInL = 19.25;
		const float xInR = 29.05;
		const float xLight = 24.17;

		//constexpr float ys = 26.5;
		constexpr float ys = 46;
		constexpr float yShift = 10;

		addParam(createParamCentered<CKSS>(mm2px(Vec(xLeft, yCvSw)), module, MultiRouterL::MODE_SWITCH));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yCvSwIn)), module, MultiRouterL::TRG_CV_INPUT));
		addParam(createParamCentered<CKSSThree>(mm2px(Vec(xDir, yDir)), module, MultiRouterL::DIR_SWITCH));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xLeft, yXfd)), module, MultiRouterL::XFD_PARAM));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xLeft, yRstKn)), module, MultiRouterL::RST_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRstIn)), module, MultiRouterL::RST_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInL, yIn)), module, MultiRouterL::IN_LEFT_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInR, yIn)), module, MultiRouterL::IN_RIGHT_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xOuts, yOuts)), module, MultiRouterL::OUTS_PARAM));

		for (int i = 0; i < 8; i++) {
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xInL, ys+(i*yShift))), module, MultiRouterL::OUT_LEFT_OUTPUT+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xInR, ys+(i*yShift))), module, MultiRouterL::OUT_RIGHT_OUTPUT+i));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xLight, ys+(i*yShift)-2.5)), module, MultiRouterL::OUT_LIGHT+i));
		}

		
	}

	void appendContextMenu(Menu* menu) override {
		MultiRouterL* module = dynamic_cast<MultiRouterL*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Cycle", "", &module->cycle));
		menu->addChild(createBoolPtrMenuItem("RST input = reverse advance", "", &module->revAdv));
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));

	}
};

Model* modelMultiRouterL = createModel<MultiRouterL, MultiRouterLWidget>("MultiRouterL");