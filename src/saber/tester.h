#ifndef SABER_TESTER_INCLUDED
#define SABER_TESTER_INCLUDED

#include <stdint.h>
#include "Grinliz_Arduino_Util.h"

class Button;
struct ButtonCBHandlers;
class Tester;
struct RGB;
class SaberDB;

class Test
{
public:
	enum {
		TEST_CONTINUE,
		TEST_ERROR,
		TEST_SUCCESS,
	};

	virtual const char* name() const = 0;
	virtual void start(Tester* tester)	{}
	virtual int process(Tester* tester, EventQueue* queue) = 0;

protected:
};


class Tester
{
public:
	Tester();
	void attach(Button* buttonA, Button* buttonB);
	void attachUI(const RGB* uiLEDs) { leds = uiLEDs; }
	void attachDB(SaberDB* _saberDB) { saberDB = _saberDB; }

	void runTests();
	void process();
	static Tester* instance() { return s_instance; }

	// interface for Test classes to call:
	void delay(uint32_t time);
	void fire(const char* event);
	void press(int button, uint32_t time);
	void delayedPress(int button, uint32_t wait, uint32_t time);
	uint32_t getRandom() { return r.rand16(); }
	const RGB* getLEDs() const { return leds; }
	SaberDB* getSaberDB() { return saberDB; }

	int getOrder() const { return order; }
	void incrementOrder() { order++; }

private:	
	void start();
	void done();

	enum {
		TEST_STATE_NONE,
		TEST_STATE_START,
		TEST_STATE_RUN,
		TEST_STATE_DONE
	};

	int currentTest = 0;
	bool running = false;
	uint32_t delayTime = 0;
	const RGB* leds = 0;
	SaberDB* saberDB = 0;
	int order = 0;

	struct Press {
		uint32_t start;
		uint32_t end;
		void reset() { start = end = 0; }
	};
	Random r;
	Press pressState[2];
	Button* button[2];

	static Tester* s_instance;
};

#endif // SABER_TESTER_INCLUDED
