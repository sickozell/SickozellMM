#define SAMPLE_HOLD 1
#define TRACK_HOLD 0
#define FULL_NOISE 0

#include "plugin.hpp"

using namespace std;

struct HolderL : Module {
	enum ParamId {
		MODE_SWITCH,
		PROB_PARAM,
		PROBATNV_PARAM,
		SCALE_PARAM,
		SCALEATNV_PARAM,
		OFFSET_PARAM,
		OFFSETATNV_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIG_INPUT,
		PROB_INPUT,
		SCALE_INPUT,
		OFFSET_INPUT,
		IN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		TRIG_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	int mode = SAMPLE_HOLD;
	int noiseType = FULL_NOISE;
	
	//float trigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//float prevTrigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float trigValue = 0;
	float prevTrigValue = 0;
	
	bool trigOnStart = true;
	bool trigOnEnd = true;
	int sampleOnGate = 0;
	bool gateOnTH = false;

	float out = 0.f;
	//int chan = 1;
	//int chanTrig = 1;

	float probSetup = 1.f;
	float probValue = 0.f;

	//bool holding[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool holding = false;

	float oneMsSamples = (APP->engine->getSampleRate()) / 1000;			// samples in 1ms
	//float outTrigSample[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//float outTrig[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	float outTrigSample = 0;
	bool outTrig = false;

	//**************************************************************
	//  DEBUG 

	/*
	std::string debugDisplay = "X";
	std::string debugDisplay2 = "X";
	std::string debugDisplay3 = "X";
	std::string debugDisplay4 = "X";
	int debugInt = 0;
	bool debugBool = false;
	*/

	HolderL() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Track & Hold", "Sample & Hold"});
		configInput(TRIG_INPUT, "Trig/Gate");
		configInput(IN_INPUT, "Signal");

		configParam(PROB_PARAM, 0, 1.f, 1.f, "Probability", "%", 0, 100);
		configParam(PROBATNV_PARAM, -1.f, 1.f, 0.f, "Probability CV", "%", 0, 100);
		configInput(PROB_INPUT, "Probability");

		configParam(SCALE_PARAM, -1.f, 1.f, 1.f, "Scale", "%", 0, 100);
		configParam(SCALEATNV_PARAM, -1.f, 1.f, 0.f, "Scale CV", "%", 0, 100);
		configInput(SCALE_INPUT, "Scale");

		configParam(OFFSET_PARAM, -10.f, 10.f, 0.f, "Offset", "v");
		configParam(OFFSETATNV_PARAM, -1.f, 1.f, 0.f, "Offset CV", "%", 0, 100);
		configInput(OFFSET_INPUT, "Offset");

		configOutput(OUT_OUTPUT, "Signal");
		configOutput(TRIG_OUTPUT, "Gate");
	}

	void onReset(const ResetEvent &e) override {

		mode = SAMPLE_HOLD;

		/*for (int i = 0; i < 16; i++) {
			trigValue[i] = 0;
			prevTrigValue[i] = 0;
			outputs[OUT_OUTPUT].setVoltage(0, i);
		}*/
		trigValue = 0;
		prevTrigValue = 0;
		outputs[OUT_OUTPUT].setVoltage(0);

		trigOnStart = true;
		trigOnEnd = true;
		sampleOnGate = 0;
		gateOnTH = false;

		noiseType = FULL_NOISE;

		//outputs[OUT_OUTPUT].setChannels(1);

		Module::onReset(e);
	}

	void onSampleRateChange() override {

		oneMsSamples = APP->engine->getSampleRate() / 1000;

	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "noiseType", json_boolean(noiseType));
		json_object_set_new(rootJ, "sampleOnGate", json_integer(sampleOnGate));
		json_object_set_new(rootJ, "gateOnTH", json_boolean(gateOnTH));
		json_object_set_new(rootJ, "trigOnStart", json_boolean(trigOnStart));
		json_object_set_new(rootJ, "trigOnEnd", json_boolean(trigOnEnd));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* noiseTypeJ = json_object_get(rootJ, "noiseType");
		if (noiseTypeJ)
			noiseType = json_boolean_value(noiseTypeJ);

		json_t* sampleOnGateJ = json_object_get(rootJ, "sampleOnGate");
		if (sampleOnGateJ)
			sampleOnGate = json_integer_value(sampleOnGateJ);

		json_t* gateOnTHJ = json_object_get(rootJ, "gateOnTH");
		if (gateOnTHJ)
			gateOnTH = json_boolean_value(gateOnTHJ);

		json_t* trigOnStartJ = json_object_get(rootJ, "trigOnStart");
		if (trigOnStartJ)
			trigOnStart = json_boolean_value(trigOnStartJ);

		json_t* trigOnEndJ = json_object_get(rootJ, "trigOnEnd");
		if (trigOnEndJ)
			trigOnEnd = json_boolean_value(trigOnEndJ);

	}

	bool isSampleOnHighGate() {
		return sampleOnGate;
	}

	void setSampleOnGate(bool sampleOnHighGate) {
		if (sampleOnHighGate) {
			sampleOnGate = 1;
			/*for (int i = 0; i < 16; i++)
				holding[i] = true;*/
			holding = true;
		} else {
			sampleOnGate = 0;
			/*for (int i = 0; i < 16; i++)
				holding[i] = false;*/
			holding = false;
		}
	}

	bool isGateOut() {
		return gateOnTH;
	}

	void setGateOut(bool gateOut) {
		if (gateOut)
			gateOnTH = true;
		else
			gateOnTH = false;

		/*for (int i = 0; i < 16; i++)
			outputs[TRIG_OUTPUT].setVoltage(0.f, i);*/
		outputs[TRIG_OUTPUT].setVoltage(0.f);

		//outputs[TRIG_OUTPUT].setChannels(1);
	}

	void setNoisePreset() {
		params[MODE_SWITCH].setValue(TRACK_HOLD);
		params[SCALE_PARAM].setValue(1);
		params[SCALEATNV_PARAM].setValue(0);
		params[OFFSET_PARAM].setValue(0);
		params[OFFSETATNV_PARAM].setValue(0);
		sampleOnGate = 1;
	}

	void process(const ProcessArgs& args) override {

		mode = params[MODE_SWITCH].getValue();

		if (outputs[OUT_OUTPUT].isConnected()) {
			
			probSetup = params[PROB_PARAM].getValue() + (inputs[PROB_INPUT].getVoltage() * params[PROBATNV_PARAM].getValue() * .1);
			if (probSetup > 1)
				probSetup = 1;
			else if (probSetup < 0)
				probSetup = 0;

			switch (mode) {
				case SAMPLE_HOLD:
					if (inputs[IN_INPUT].isConnected()) {
						
						//chan = inputs[IN_INPUT].getChannels();

						//chanTrig = inputs[TRIG_INPUT].getChannels();

						//if (chanTrig == 1) {	// SAMPLE & HOLD - mono Trigger

							trigValue = inputs[TRIG_INPUT].getVoltage();

							if (trigValue >= 1.f && prevTrigValue < 1.f) {
								probValue = random::uniform();
								if (probSetup >= probValue) {
									//for (int c = 0; c < chan; c++) {

										out = ( inputs[IN_INPUT].getVoltage() *
											(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
											(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT].setVoltage(out);
									//}
									outTrig = true;
									outTrigSample = oneMsSamples;
								}					
								
							}
							prevTrigValue = trigValue;
							//outputs[OUT_OUTPUT].setChannels(chan);

						/*} else { // poly Trigger

							for (int cT = 0; cT < chanTrig; cT++) {
								trigValue[cT] = inputs[TRIG_INPUT].getVoltage(cT);
								if (trigValue[cT] >= 1.f && prevTrigValue[cT] < 1.f) {
									probValue = random::uniform();
									if (probSetup >= probValue) {
										out = ( inputs[IN_INPUT].getVoltage(cT) *
											(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
											(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT].setVoltage(out, cT);
									}
									outTrig[cT] = true;
									outTrigSample[cT] = oneMsSamples;
								}
								prevTrigValue[cT] = trigValue[cT];
							}
							outputs[OUT_OUTPUT].setChannels(chanTrig);
						}*/
						
					} else {	// Sample & Hold with noise generator
						
						//chanTrig = inputs[TRIG_INPUT].getChannels();

						//for (int cT = 0; cT < chanTrig; cT++) {
							trigValue = inputs[TRIG_INPUT].getVoltage();
							if (trigValue >= 1.f && prevTrigValue < 1.f) {
								probValue = random::uniform();
								if (probSetup >= probValue) { 

									if (noiseType == FULL_NOISE) 
										out = ( (random::uniform() * 10 - 5.f) *
												(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
												(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );
									else
										out = ( random::normal() *
												(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
												(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

									if (out > 10.f)
										out = 10.f;
									else if (out < -10.f)
										out = -10.f;

									outputs[OUT_OUTPUT].setVoltage(out);
									outTrig = true;
									outTrigSample = oneMsSamples;
								}
							}
							prevTrigValue = trigValue;
						//}
						//outputs[OUT_OUTPUT].setChannels(chanTrig);

					}
				
				break;

				case TRACK_HOLD:
					switch (sampleOnGate) {
						case 0:	// sample on LOW GATE
							if (inputs[IN_INPUT].isConnected()) {

								//chan = inputs[IN_INPUT].getChannels();
								//chanTrig = inputs[TRIG_INPUT].getChannels();

								//if (chanTrig == 1) {	// monophonic trigger

									trigValue = inputs[TRIG_INPUT].getVoltage();

									if (trigValue >= 1.f && prevTrigValue < 1.f) {
										probValue = random::uniform();
										if (probSetup >= probValue) {
											//for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage() *
														(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
														(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT].setVoltage(out);

											//}

											if (!gateOnTH && trigOnStart) {
												outTrig = true;
												outTrigSample = oneMsSamples;
											}

											holding = false;
										}
									} else if (trigValue >= 1.f && !holding) {
										//for (int c = 0; c < chan; c++) {

											out = ( inputs[IN_INPUT].getVoltage() *
													(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
													(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out);

										//}

										if (!gateOnTH && trigOnStart && prevTrigValue < 1.f) {
											outTrig = true;
											outTrigSample = oneMsSamples;
										}

									} else if (trigValue < 1) {
										if (!holding) {
											holding = true;
											//for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage() *
														(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
														(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT].setVoltage(out);

											//}

											if (!gateOnTH && trigOnEnd) {
												outTrig = true;
												outTrigSample = oneMsSamples;
											}
										}
									}

									prevTrigValue = trigValue;
									//outputs[OUT_OUTPUT].setChannels(chan);

									if (gateOnTH) {
										if (holding) {
											outputs[TRIG_OUTPUT].setVoltage(0.f);
										} else {
											outputs[TRIG_OUTPUT].setVoltage(10.f);
										}
									}
									//outputs[TRIG_OUTPUT].setChannels(1);

								/*} else {	// TRACK & HOLD  SAMPLE ON LOW GATE: polyphonic Triggers

									for (int cT = 0; cT < chanTrig; cT++) {
										trigValue[cT] = inputs[TRIG_INPUT].getVoltage(cT);

										if (trigValue[cT] >= 1.f && prevTrigValue[cT] < 1.f) {
											probValue = random::uniform();
											if (probSetup >= probValue) {
												//for (int c = 0; c < chan; c++) {

													out = ( inputs[IN_INPUT].getVoltage(cT) *
															(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
															(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

													if (out > 10.f)
														out = 10.f;
													else if (out < -10.f)
														out = -10.f;

													outputs[OUT_OUTPUT].setVoltage(out, cT);

												//}

												if (!gateOnTH && trigOnStart) {
													outTrig[cT] = true;
													outTrigSample[cT] = oneMsSamples;
												}

												holding[cT] = false;
											}
										} else if (trigValue[cT] >= 1.f && !holding[cT]) {
											//for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage(cT) *
														(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
														(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT].setVoltage(out, cT);

											//}

											if (!gateOnTH && trigOnStart && prevTrigValue[cT] < 1.f) {
												outTrig[cT] = true;
												outTrigSample[cT] = oneMsSamples;
											}

										} else if (trigValue[cT] < 1) {
											if (!holding[cT]) {
												holding[cT] = true;
												//for (int c = 0; c < chan; c++) {

													out = ( inputs[IN_INPUT].getVoltage(cT) *
															(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
															(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

													if (out > 10.f)
														out = 10.f;
													else if (out < -10.f)
														out = -10.f;

													outputs[OUT_OUTPUT].setVoltage(out, cT);

												//}

												if (!gateOnTH && trigOnEnd) {
													outTrig[cT] = true;
													outTrigSample[cT] = oneMsSamples;
												}
											}
										}

										prevTrigValue[cT] = trigValue[cT];
										

										if (gateOnTH) {
											if (holding[cT]) {
												outputs[TRIG_OUTPUT].setVoltage(0.f, cT);
											} else {
												outputs[TRIG_OUTPUT].setVoltage(10.f, cT);
											}
										}
									}
									outputs[TRIG_OUTPUT].setChannels(chanTrig);
									outputs[OUT_OUTPUT].setChannels(chanTrig);

								}*/

							} else {	// TRACK & HOLD - SAMPLE ON LOW GATE - with Noise Generator

								//chanTrig = inputs[TRIG_INPUT].getChannels();

								//for (int cT = 0; cT < chanTrig; cT++) {

									trigValue = inputs[TRIG_INPUT].getVoltage();

									if (trigValue >= 1.f && prevTrigValue < 1.f) {
										probValue = random::uniform();
										if (probSetup >= probValue) {
											if (noiseType == FULL_NOISE) 
												out = ( (random::uniform() * 10 - 5.f) *
														(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
														(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );
											else
												out = ( random::normal() *
														(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
														(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out);

											if (!gateOnTH && trigOnStart) {
												outTrig = true;
												outTrigSample = oneMsSamples;
											}

											holding = false;
										}
									} else if (trigValue >= 1.f && !holding) {

										if (noiseType == FULL_NOISE) 
											out = ( (random::uniform() * 10 - 5.f) *
													(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
													(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );
										else
											out = ( random::normal() *
													(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
													(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT].setVoltage(out);

										if (!gateOnTH && trigOnStart && prevTrigValue < 1.f) {
											outTrig = true;
											outTrigSample = oneMsSamples;
										}

									} else if (trigValue < 1) {
										if (!holding) {
											holding = true;

											if (noiseType == FULL_NOISE) 
												out = ( (random::uniform() * 10 - 5.f) *
														(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
														(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );
											else
												out = ( random::normal() *
														(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
														(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out);

											if (!gateOnTH && trigOnEnd) {
												outTrig = true;
												outTrigSample = oneMsSamples;
											}
										}
									}

									prevTrigValue = trigValue;
									
									if (gateOnTH) {
										if (holding) {
											outputs[TRIG_OUTPUT].setVoltage(0.f);
										} else {
											outputs[TRIG_OUTPUT].setVoltage(10.f);
										}
									}
									
								//}
								//outputs[OUT_OUTPUT].setChannels(chanTrig);
								//outputs[TRIG_OUTPUT].setChannels(chanTrig);
							}

						break;

						case 1:		// sample on HIGH GATE	
							if (inputs[IN_INPUT].isConnected()) {

								//chan = inputs[IN_INPUT].getChannels();
								//chanTrig = inputs[TRIG_INPUT].getChannels();

								//if (chanTrig == 1) {	// TRACK & HOLD - SAMPLE ON HIGH GATE - monophonic trigger

									trigValue = inputs[TRIG_INPUT].getVoltage();

									if (trigValue >= 1.f && prevTrigValue < 1.f) {
										probValue = random::uniform();
										if (probSetup >= probValue) {
											holding = true;
											//for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage() *
													(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
													(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;
												outputs[OUT_OUTPUT].setVoltage(out);
											//}

											if (gateOnTH) {
												outputs[TRIG_OUTPUT].setVoltage(10.f);
											} else {
												if (trigOnStart) {
													outTrig = true;
													outTrigSample = oneMsSamples;
												}
											}
										} else {
											holding = false;
										}
									} else if (trigValue < 1.f) {

										if (gateOnTH) {
											outputs[TRIG_OUTPUT].setVoltage(0.f);
										} else {
											if (holding && trigOnEnd) {
												outTrig = true;
												outTrigSample = oneMsSamples;
											}
										}
										//outputs[TRIG_OUTPUT].setChannels(1);

										holding = false;
									}
									prevTrigValue = trigValue;

									if (!holding) {
										//for (int c = 0; c < chan; c++) {

											out = ( inputs[IN_INPUT].getVoltage() *
													(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
													(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out);
										//}							
									}

									//outputs[OUT_OUTPUT].setChannels(chan);
								
								/*} else {	// TRACK & HOLD - SAMPLE ON HIGH GATE - polyphonic triggers

									for (int cT = 0; cT < chanTrig; cT++) {

										trigValue[cT] = inputs[TRIG_INPUT].getVoltage(cT);

										if (trigValue[cT] >= 1.f && prevTrigValue[cT] < 1.f) {
											probValue = random::uniform();
											if (probSetup >= probValue) {
												holding[cT] = true;
												//for (int c = 0; c < chan; c++) {

													out = ( inputs[IN_INPUT].getVoltage(cT) *
														(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
														(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

													if (out > 10.f)
														out = 10.f;
													else if (out < -10.f)
														out = -10.f;
													outputs[OUT_OUTPUT].setVoltage(out, cT);
												//}

												if (gateOnTH) {
													outputs[TRIG_OUTPUT].setVoltage(10.f, cT);
												} else {
													if (trigOnStart) {
														outTrig[cT] = true;
														outTrigSample[cT] = oneMsSamples;
													}
												}
											} else {
												holding[cT] = false;
											}
										} else if (trigValue[cT] < 1.f) {

											if (gateOnTH) {
												outputs[TRIG_OUTPUT].setVoltage(0.f, cT);
											} else {
												if (holding[cT] && trigOnEnd) {
													outTrig[cT] = true;
													outTrigSample[cT] = oneMsSamples;
												}
											}

											holding[cT] = false;
										}
										prevTrigValue[cT] = trigValue[cT];

										if (!holding[cT]) {
											//for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage(cT) *
														(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
														(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT].setVoltage(out, cT);
											//}							
										}
									}

									outputs[OUT_OUTPUT].setChannels(chanTrig);
									outputs[TRIG_OUTPUT].setChannels(chanTrig);

								}*/
							} else {	// TRACK & HOLD - Sample on HIGH gate - noise generator

								//chanTrig = inputs[TRIG_INPUT].getChannels();

								//if (chanTrig == 0)
								//	chanTrig = 1;

								//for (int cT = 0; cT < chanTrig; cT++) {

									if (noiseType == FULL_NOISE) 
										out = ( (random::uniform() * 10 - 5.f) *
												(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
												(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );
									else
										out = ( random::normal() *
												(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * params[SCALEATNV_PARAM].getValue() * .1)) ) +
												(params[OFFSET_PARAM].getValue() + (inputs[OFFSET_INPUT].getVoltage() * params[OFFSETATNV_PARAM].getValue()) );

									trigValue = inputs[TRIG_INPUT].getVoltage();
									if (trigValue >= 1.f && prevTrigValue < 1.f) {
										probValue = random::uniform();
										if (probSetup >= probValue) {
											holding = true;
											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out);
											if (gateOnTH) {
												outputs[TRIG_OUTPUT].setVoltage(10.f);
											} else {
												if (trigOnStart) {
													outTrig = true;
													outTrigSample = oneMsSamples;
												}
											}
										} else {
											holding = false;
										}
									} else if (trigValue < 1.f) {

										if (gateOnTH) {
											outputs[TRIG_OUTPUT].setVoltage(0.f);
										} else {
											if (holding && trigOnEnd) {
												outTrig = true;
												outTrigSample = oneMsSamples;
											}
										}
										holding = false;
									}
									prevTrigValue = trigValue;

									if (!holding) {
										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT].setVoltage(out);
									}
								//}
								//outputs[OUT_OUTPUT].setChannels(chanTrig);
								//outputs[TRIG_OUTPUT].setChannels(chanTrig);
							}
						break;
					}	
				break;
			}
		} else {
			outputs[OUT_OUTPUT].setVoltage(0.f);
			//outputs[OUT_OUTPUT].setChannels(1);
			outputs[TRIG_OUTPUT].setVoltage(0.f);
			//outputs[TRIG_OUTPUT].setChannels(1);
		}

		//for (int c = 0; c < 16; c++) {
			if (outTrig) {
				outTrigSample--;
				if (outTrigSample > 0) {
					outputs[TRIG_OUTPUT].setVoltage(10.f);
				} else {
					outTrig = false;
					outputs[TRIG_OUTPUT].setVoltage(0.f);
				}
			}
		//}
	}
};

struct HolderLDebugDisplay : TransparentWidget {
	HolderL *module;
	int frame = 0;
	HolderLDebugDisplay() {
	}

	/*
	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 
				
				nvgTextBox(args.vg, 9, 6,120, module->debugDisplay.c_str(), NULL);
				//nvgTextBox(args.vg, 9, 16,120, module->debugDisplay2.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 6,120, module->debugDisplay3.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 16,120, module->debugDisplay4.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
	*/
};

struct HolderLWidget : ModuleWidget {
	HolderLWidget(HolderL* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/HolderL.svg")));

		/*
		{
			HolderLDebugDisplay *display = new HolderLDebugDisplay();
			display->box.pos = Vec(23, 3);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		/*
		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		*/

		addParam(createParamCentered<CKSS>(mm2px(Vec(9.958, 18.65)), module, HolderL::MODE_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(25.5, 18.65)), module, HolderL::TRIG_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(8, 42.7)), module, HolderL::PROB_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(18.4, 42.7)), module, HolderL::PROBATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(28.1, 42.7)), module, HolderL::PROB_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(10.48, 64.9)), module, HolderL::SCALE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(10.48, 76.5)), module, HolderL::SCALEATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(10.48, 85.2)), module, HolderL::SCALE_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(25.1, 64.9)), module, HolderL::OFFSET_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(25.1, 76.5)), module, HolderL::OFFSETATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(25.1, 85.5)), module, HolderL::OFFSET_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(9.5, 108.8)), module, HolderL::IN_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(26.3, 103)), module, HolderL::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(26.3, 116.5)), module, HolderL::TRIG_OUTPUT));

	}

	void appendContextMenu(Menu* menu) override {
		HolderL* module = dynamic_cast<HolderL*>(this->module);

		struct ModeItem : MenuItem {
			HolderL* module;
			int noiseType;
			void onAction(const event::Action& e) override {
				module->noiseType = noiseType;
			}
		};

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("White Noise Type"));
		std::string modeNames[2] = {"Full", "Centered"};
		for (int i = 0; i < 2; i++) {
			ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
			modeItem->rightText = CHECKMARK(module->noiseType == i);
			modeItem->module = module;
			modeItem->noiseType = i;
			menu->addChild(modeItem);
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Track & Hold options:"));

		menu->addChild(createBoolMenuItem("Sample on HIGH Gate", "", [=]() {
					return module->isSampleOnHighGate();
				}, [=](bool sampleOnHighGate) {
					module->setSampleOnGate(sampleOnHighGate);
			}));

		if (module->gateOnTH == true) {
			menu->addChild(createMenuLabel("Trig on start"));
			menu->addChild(createMenuLabel("Trig on end"));
		} else {
			menu->addChild(createBoolPtrMenuItem("Trig on start", "", &module->trigOnStart));
			menu->addChild(createBoolPtrMenuItem("Trig on end", "", &module->trigOnEnd));
		}

		menu->addChild(createBoolMenuItem("Gate Out instead Trig", "", [=]() {
					return module->isGateOut();
				}, [=](bool gateOut) {
					module->setGateOut(gateOut);
			}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Noise Generator preset", "", [=]() {module->setNoisePreset();}));
	}
};

Model* modelHolderL = createModel<HolderL, HolderLWidget>("HolderL");