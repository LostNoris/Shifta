#include "plugin.hpp"
#include <vector> 

struct ShiftRegister : Module {


	dsp::SchmittTrigger Trigger;

	float values[64];

	static const int SIZE = 64;

	std::vector<std::pair<int, int>> regions;

	int regionLengths[4] = { 16,16,16,16 };

	enum ParamId {
		SAMPLECHANCE_PARAM,

		REGION_1_LENGTH,
		REGION_2_LENGTH,
		REGION_3_LENGTH,
		REGION_4_LENGTH,

		POSITION_1,
		POSITION_2,
		POSITION_3,
		POSITION_4,

		PARAMS_LEN
	};

	enum InputId {
		CLOCK_INPUT,

		SAMPLE_CV_INPUT,

		REGION_1_LENGTH_CV,
		REGION_2_LENGTH_CV,
		REGION_3_LENGTH_CV,
		REGION_4_LENGTH_CV,

		POSITION_1_CV,
		POSITION_2_CV,
		POSITION_3_CV,
		POSITION_4_CV,

		INPUTS_LEN
	};

	enum OutputId {
		OUT_1,
		OUT_2,
		OUT_3,
		OUT_4, 
		OUTPUTS_LEN
	};

	enum LightId {
		SEQ_LIGHT,
		SEQ_LIGHT_LAST = SEQ_LIGHT + 3,
		LIGHTS_LEN
	};

	ShiftRegister() {

		for (int i = 0; i < SIZE; i++) {
			values[i] = 0.f;
		}

		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

	    configParam(SAMPLECHANCE_PARAM, 0.f, 100.f, 0.f, "Sample Chance");

		configParam(REGION_1_LENGTH, 1.f, 64.f, 16.f, "Region 1 Size");
		paramQuantities[REGION_1_LENGTH]->snapEnabled = true;
		configParam(REGION_2_LENGTH, 1.f, 64.f, 16.f, "Region 2 Size");
		paramQuantities[REGION_2_LENGTH]->snapEnabled = true;
		configParam(REGION_3_LENGTH, 1.f, 64.f, 16.f, "Region 3 Size");
		paramQuantities[REGION_3_LENGTH]->snapEnabled = true;
		configParam(REGION_4_LENGTH, 1.f, 64.f, 16.f, "Region 4 Size");
		paramQuantities[REGION_4_LENGTH]->snapEnabled = true;

		configParam(POSITION_1, 0.f, 63.f, 0.f, "Sequence Position 1");
		paramQuantities[POSITION_1]->snapEnabled = true;
		configParam(POSITION_2, 0.f, 63.f, 16.f, "Sequence Position 2");
		paramQuantities[POSITION_2]->snapEnabled = true;
		configParam(POSITION_3, 0.f, 63.f, 32.f, "Sequence Position 3");
		paramQuantities[POSITION_3]->snapEnabled = true;
		configParam(POSITION_4, 0.f, 63.f, 48.f, "Sequence Position 4");
		paramQuantities[POSITION_4]->snapEnabled = true;

		configInput(CLOCK_INPUT, "Clock");
		
		configInput(SAMPLE_CV_INPUT, "Sample and Hold Signal");

		for (int i = 0; i < 4; i++) {
			configInput(REGION_1_LENGTH_CV + i, "Region CV");
			configInput(POSITION_1_CV + i, "Position CV");
		}
		
		configOutput(OUT_1, "sequence position 1");
		configOutput(OUT_2, "sequence position 2");
		configOutput(OUT_3, "sequence position 3");
		configOutput(OUT_4, "sequence position 4");

	}

	void buildRegions() {
		regions.clear();

		int start = 0;

		for (int i = 0; i < 4; i++) {

			int len = regionLengths[i];

			int remaining = SIZE - start;

			if (remaining <= 0)
				break;

			len = std::min(len, remaining);

			int end = start + len - 1;

			regions.push_back({ start, end });

			start = end + 1;
		}
	}

	void shiftRegion(int start, int end) {

		float last = values[end];

		for (int i = end; i > start; i--) {
			values[i] = values[i - 1];
		}

		values[start] = last;
	}

	void process(const ProcessArgs& args) override {

		int pos1 = params[POSITION_1].getValue();
		int pos2 = params[POSITION_2].getValue();
		int pos3 = params[POSITION_3].getValue();
		int pos4 = params[POSITION_4].getValue();

		float pos1cv = inputs[POSITION_1_CV].isConnected()
			? inputs[POSITION_1_CV].getVoltage()
			: 0.f;
		pos1 = clamp((int)std::round(pos1 + pos1cv), 1, 64);

		float pos2cv = inputs[POSITION_2_CV].isConnected()
			? inputs[POSITION_2_CV].getVoltage()
			: 0.f;
		pos2 = clamp((int)std::round(pos2 + pos2cv), 1, 64);

		float pos3cv = inputs[POSITION_3_CV].isConnected()
			? inputs[POSITION_3_CV].getVoltage()
			: 0.f;
		pos3 = clamp((int)std::round(pos3 + pos3cv), 1, 64);

		float pos4cv = inputs[POSITION_4_CV].isConnected()
			? inputs[POSITION_4_CV].getVoltage()
			: 0.f;
		pos4 = clamp((int)std::round(pos4 + pos4cv), 1, 64);

		if (Trigger.process(inputs[CLOCK_INPUT].getVoltage(),0.1f,2.0f)) {

			float region1cv = inputs[REGION_1_LENGTH_CV].isConnected()
				? inputs[REGION_1_LENGTH_CV].getVoltage()
				: 0.f;
			float region1knob = params[REGION_1_LENGTH].getValue();
			int region1len = clamp((int)std::round(region1knob + region1cv), 1, 64);
			regionLengths[0] = region1len;

			float region2cv = inputs[REGION_2_LENGTH_CV].isConnected()
				? inputs[REGION_2_LENGTH_CV].getVoltage()
				: 0.f;
			float region2knob = params[REGION_2_LENGTH].getValue();
			int region2len = clamp((int)std::round(region2knob + region2cv), 1, 64);
			regionLengths[1] = region2len;

			float region3cv = inputs[REGION_3_LENGTH_CV].isConnected()
				? inputs[REGION_3_LENGTH_CV].getVoltage()
				: 0.f;
			float region3knob = params[REGION_3_LENGTH].getValue();
			int region3len = clamp((int)std::round(region3knob + region3cv), 1, 64);
			regionLengths[2] = region3len;

			float region4cv = inputs[REGION_4_LENGTH_CV].isConnected()
				? inputs[REGION_4_LENGTH_CV].getVoltage()
				: 0.f;
			float region4knob = params[REGION_4_LENGTH].getValue();
			int region4len = clamp((int)std::round(region4knob + region4cv), 1, 64);
			regionLengths[3] = region4len;

			buildRegions();

			float probability = params[SAMPLECHANCE_PARAM].getValue() / 100.f;

			if (random::uniform() < probability) {

				// SAMPLE MODE

				for (int i = SIZE - 1; i > 0; i--) {
					values[i] = values[i - 1];
				}

				values[0] = inputs[SAMPLE_CV_INPUT].getVoltage();

			}
			else {

				// SHIFT MODE

				for (auto region : regions) {
					shiftRegion(region.first, region.second);
				}
			}
			
		}

		float reg1 = values[pos1];
		float reg2 = values[pos2];
		float reg3 = values[pos3];
		float reg4 = values[pos4];

		outputs[OUT_1].setVoltage(reg1);
		outputs[OUT_2].setVoltage(reg2);
		outputs[OUT_3].setVoltage(reg3);
		outputs[OUT_4].setVoltage(reg4);

	}
};


struct ShiftRegisterWidget : ModuleWidget {
	ShiftRegisterWidget(ShiftRegister* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Sequencer.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(18, 25)),
			module,
			ShiftRegister::CLOCK_INPUT
		));

		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(18, 35)),
			module,
			ShiftRegister::SAMPLE_CV_INPUT
		));

		for (int i = 0; i < 4; i++) {
			float x = 18.f + i * 15.f;
			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(x, 95)),
				module,
				ShiftRegister::REGION_1_LENGTH_CV + i
			));
		}

		for (int i = 0; i < 4; i++) {
			float x = 18.f + i * 15.f;
			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(x, 65)),
				module,
				ShiftRegister::POSITION_1_CV + i
			));
		}

		addParam(createParamCentered<RoundBlackKnob>(
			mm2px(Vec(63, 35)),
			module,
			ShiftRegister::SAMPLECHANCE_PARAM
		));

		for (int i = 0; i < 4; i++) {
			float x = 18.f + i * 15.f;
			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 55)),
				module,
				ShiftRegister::POSITION_1 + i
			));
		}

		for (int i = 0; i < 4; i++) {
			float x = 18.f + i * 15.f;
			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 85)),
				module,
				ShiftRegister::REGION_1_LENGTH + i
			));
		}


		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18, 120)), module, ShiftRegister::OUT_1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33, 120)), module, ShiftRegister::OUT_2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48, 120)), module, ShiftRegister::OUT_3));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(63, 120)), module, ShiftRegister::OUT_4));


	}
};


Model* modelShiftRegisterSequencer = createModel<ShiftRegister, ShiftRegisterWidget>("ShiftRegisterSequencer");