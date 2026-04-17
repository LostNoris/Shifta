#include "plugin.hpp"


struct DifferentialSequencer : Module {

	int Lerpstep = 0;
	int Sumstep = 0;
	int Diffstep = 0;
	int Prodstep = 0;

	dsp::SchmittTrigger lerpTrigger;
	dsp::SchmittTrigger sumTrigger;
	dsp::SchmittTrigger diffTrigger;
	dsp::SchmittTrigger prodTrigger;

	float values[8];
	float differences[8];
	float sums[8];
	float products[8];
	float sequenceOut[8];

	enum ParamId {
		VALUE_PARAM,
		VALUE_PARAM_LAST = VALUE_PARAM + 7,

		TARGET_PARAM,
		TARGET_PARAM_LAST = TARGET_PARAM + 7,

		MORPH_PARAM,
		MORPH_PARAM_LAST = MORPH_PARAM + 7,

		LENGTH_PARAM,
		LENGTH_PARAM_LAST = LENGTH_PARAM + 3,

		PARAMS_LEN
	};

	enum InputId {
		LERPCLOCK_INPUT,
		SUMCLOCK_INPUT,
		DIFFCLOCK_INPUT,
		PRODCLOCK_INPUT,

		VALUE_CV_INPUT,
		VALUE_CV_INPUT_LAST = VALUE_CV_INPUT + 7,

		INPUTS_LEN
	};

	enum OutputId {
		LERP_OUTPUT,
		SUM_OUTPUT,
		DIFF_OUTPUT,
		PROD_OUTPUT,
		OUTPUTS_LEN
	};

	enum LightId {
		SEQ_LIGHT,
		SEQ_LIGHT_LAST = SEQ_LIGHT + 3,
		LIGHTS_LEN
	};

	DifferentialSequencer() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		for (int i = 0; i < 8; i++) {

			configParam(VALUE_PARAM + i, -5.f, 5.f, 0.f, "Step value");

			configParam(TARGET_PARAM + i, 0.f, 7.f, 0.f, "Target step");
			paramQuantities[TARGET_PARAM + i]->snapEnabled = true;

			configParam(MORPH_PARAM + i, 0.f, 1.f, 0.f, "Operator morph");
		}

		for (int i = 0; i < 4; i++) {
		configParam(LENGTH_PARAM + i, -8.f, 8.f, 0.f, "Sequence Length");
		paramQuantities[LENGTH_PARAM + i]->snapEnabled = true;
	    }


		configInput(LERPCLOCK_INPUT, "Lerped Clock");
		configInput(SUMCLOCK_INPUT, "Summed Clock");
		configInput(DIFFCLOCK_INPUT, "Difference Clock");
		configInput(PRODCLOCK_INPUT, "Product Clock");

		configOutput(LERP_OUTPUT, "Lerped sequence");
		configOutput(SUM_OUTPUT, "Sum sequence");
		configOutput(DIFF_OUTPUT, "Difference sequence");
		configOutput(PROD_OUTPUT, "Product sequence");
	}

	void process(const ProcessArgs& args) override {

		if (lerpTrigger.process(inputs[LERPCLOCK_INPUT].getVoltage())) {

			int rLen = (int)params[LENGTH_PARAM].getValue();
			if (rLen == 0) rLen = 1;

			int direction = (rLen > 0) ? 1 : -1;
			int len = std::abs(rLen);

			Lerpstep = (Lerpstep + direction + len) % len;

			lights[SEQ_LIGHT].setBrightness(Lerpstep / 7.f);
		}
		if (sumTrigger.process(inputs[SUMCLOCK_INPUT].getVoltage())) {

			int rLen = (int)params[LENGTH_PARAM + 1].getValue();
			if (rLen == 0) rLen = 1;

			int direction = (rLen > 0) ? 1 : -1;
			int len = std::abs(rLen);

			Sumstep = (Sumstep + direction + len) % len;

			lights[SEQ_LIGHT + 1].setBrightness(Sumstep / 7.f);
		}
		if (diffTrigger.process(inputs[DIFFCLOCK_INPUT].getVoltage())) {

			int rLen = (int)params[LENGTH_PARAM + 2].getValue();
			if (rLen == 0) rLen = 1;

			int direction = (rLen > 0) ? 1 : -1;
			int len = std::abs(rLen);

			Diffstep = (Diffstep + direction + len) % len;

			lights[SEQ_LIGHT + 2].setBrightness(Diffstep / 7.f);
		}
		if (prodTrigger.process(inputs[PRODCLOCK_INPUT].getVoltage())) {

			int rLen = (int)params[LENGTH_PARAM + 3].getValue();
			if (rLen == 0) rLen = 1;

			int direction = (rLen > 0) ? 1 : -1;
			int len = std::abs(rLen);

			Prodstep = (Prodstep + direction + len) % len;

			lights[SEQ_LIGHT + 3].setBrightness(Prodstep / 7.f);
		}

		float values[8];
		float differences[8];
		float sums[8];
		float products[8];
		float sequenceOut[8];

		for (int i = 0; i < 8; i++) {
			float knob = params[VALUE_PARAM + i].getValue();

			float cv = inputs[VALUE_CV_INPUT + i].isConnected()	? inputs[VALUE_CV_INPUT + i].getVoltage(): 0.f;

			values[i] = clamp(knob + cv, -5.f, 5.f);
		}

		for (int i = 0; i < 8; i++) {

			int target = (int)std::round(params[TARGET_PARAM + i].getValue());
			target = clamp(target, 0, 7);

			float a = values[i];
			float b = values[target];

			differences[i] = fabs(a - b);
			sums[i] = (a + b) * 0.5f;
			products[i] = (a * b) / 25.f * 5.f;

			float morph = params[MORPH_PARAM + i].getValue();

			if (morph < 0.5f)
				sequenceOut[i] = crossfade(differences[i], sums[i], morph * 2.f);
			else
				sequenceOut[i] = crossfade(sums[i], products[i], (morph - 0.5f) * 2.f);
		}

		outputs[LERP_OUTPUT].setVoltage(sequenceOut[Lerpstep]);
		outputs[SUM_OUTPUT].setVoltage(sums[Sumstep]);
		outputs[DIFF_OUTPUT].setVoltage(differences[Diffstep]);
		outputs[PROD_OUTPUT].setVoltage(products[Prodstep]);

	   
	}
};


struct DifferentialSequencerWidget : ModuleWidget {
	DifferentialSequencerWidget(DifferentialSequencer* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/DifferentialSequencer.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for (int i = 0; i < 8; i++) {
			float x = 5.f + i * 10.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 25)),
				module,
				DifferentialSequencer::VALUE_PARAM + i
			));
		}

		for (int i = 0; i < 8; i++) {
			float x = 5.f + i * 10.f;

			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(x, 35)),
				module,
				DifferentialSequencer::VALUE_CV_INPUT + i
			));
		}

		for (int i = 0; i < 8; i++) {
			float x = 5.f + i * 10.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 45)),
				module,
				DifferentialSequencer::TARGET_PARAM + i
			));
		}

		for (int i = 0; i < 8; i++) {
			float x = 5.f + i * 10.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 65)),
				module,
				DifferentialSequencer::MORPH_PARAM + i
			));
		}

		for (int i = 0; i < 4; i++) {
			float x = 18.f + i * 15.f;

			addParam(createParamCentered<RoundBlackKnob>(
				mm2px(Vec(x, 85)),
				module,
				DifferentialSequencer::LENGTH_PARAM + i
			));
		}

		for (int i = 0; i < 4; i++) {
			float x = 18.f + i * 15.f;

			addChild(createLightCentered<MediumLight<BlueLight>>(
				mm2px(Vec(x, 100)), module,
				DifferentialSequencer::SEQ_LIGHT + i
			));
		}

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 110)),module,DifferentialSequencer::LERPCLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33, 110)),module,DifferentialSequencer::SUMCLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(48, 110)),module,DifferentialSequencer::DIFFCLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(63, 110)),module,DifferentialSequencer::PRODCLOCK_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18, 120)), module, DifferentialSequencer::LERP_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33, 120)), module, DifferentialSequencer::SUM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48, 120)), module, DifferentialSequencer::DIFF_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(63, 120)), module, DifferentialSequencer::PROD_OUTPUT));


	}
};


Model* modelDifferentialSequencer = createModel<DifferentialSequencer, DifferentialSequencerWidget>("DifferentialSequencer");