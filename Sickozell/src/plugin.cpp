#include "plugin.hpp"

//Plugin* pluginInstance;
//extern Plugin *pluginInstance;

#if defined(METAMODULE_BUILTIN)
extern Plugin *pluginInstance;
#else
Plugin *pluginInstance;
#endif

//void init(Plugin* p) {
//void init_Sickozell(rack::Plugin *p) {

#if defined(METAMODULE_BUILTIN)
void init_Sickozell(rack::Plugin *p) {
#else 
void init(rack::Plugin *p) {
#endif


	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
	
	
	p->addModel(modelAdder8);
	p->addModel(modelBgates);
	p->addModel(modelBlender);
	p->addModel(modelBlender8);
	p->addModel(modelBtogglerSt);
	p->addModel(modelBtogglerStCompact);
	p->addModel(modelBtoggler);
	p->addModel(modelCalcs);
	p->addModel(modelClocker2);
	p->addModel(modelCVrouter);
	p->addModel(modelCVswitcher);
	p->addModel(modelDrummer);
	p->addModel(modelDrummer4);
	p->addModel(modelDrummer4Plus);
	p->addModel(modelEnverL);
	p->addModel(modelHolderL);
	p->addModel(modelHolderCompactL);
	p->addModel(modelHolder8L);
	p->addModel(modelModulator);
	p->addModel(modelModulator7);
	p->addModel(modelModulator7Compact);
	p->addModel(modelMultiSwitcherL);
	p->addModel(modelMultiRouterL);
	p->addModel(modelMultiSwitcherML);
	p->addModel(modelMultiRouterML);
	p->addModel(modelRandLoops);
	p->addModel(modelShifter);
	p->addModel(modelSickoAmp);
	p->addModel(modelSickoCrosserL);
	p->addModel(modelSickoCrosser4L);
	p->addModel(modelSickoLooper1);
	p->addModel(modelSickoLooper1Exp);
	p->addModel(modelSickoLooper3);
	p->addModel(modelSickoQuantL);
	p->addModel(modelSickoQuant4L);
	p->addModel(modelSickoSampler2);
	p->addModel(modelSimpleSeq4);
	p->addModel(modelSlewerL);
	p->addModel(modelSwitcher);
	p->addModel(modelSwitcherSt);
	p->addModel(modelSwitcher8);
	p->addModel(modelTogglerCompact);
	p->addModel(modelTogglerMM);
	
}