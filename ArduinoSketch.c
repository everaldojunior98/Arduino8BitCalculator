//Created by Everaldo Junior in 05/19/20. Licensed by MIT

//CUSTOM CLASS
class Number
{
	public:
		//Multiplexer info
		int Clock;
		int Latch;
		int Data;

		//Number info Leds [b7, b6, b5, b4, b3, b2, b1, b0]
		bool Bits[8] = {0, 0, 0, 0, 0, 0, 0, 0};

		Number()  // Default constructor
		{

		}

		void WriteBits()
		{
			//Open data flow
			digitalWrite(Latch, LOW);

			for (int x = 0; x < sizeof(Bits); x++)
			{
				//Write value and pulse the clock
				digitalWrite(Clock, LOW);
				digitalWrite(Data, Bits[x]);
				digitalWrite(Clock, HIGH);
			}    
			 
			//Close data flow
			digitalWrite(Latch, HIGH);
		}
};

struct FullAdderResult
{
	bool Sum;
	bool Carry;
};

struct RippleCarryAdderResult
{
	bool Sum[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool Carry;
};

//VARIABLES
Number* number1;
Number* number2;
Number* result;

//ADDER METHODS
struct FullAdderResult FullAdder(bool a, bool b, bool c)
{
	/*
		Em um somador completo:
		Soma = (c xor a) xor b
		Vai1 = (a and b) or (a and c) or (b and c)
	*/
	
	struct FullAdderResult adderResult;

	adderResult.Sum = (c ^ a) ^ b;
	adderResult.Carry = (a && b) || (a && c) || (b && c);

	return adderResult;
}

//Generate prototype (to avoid tinkercad error)
struct RippleCarryAdderResult RippleCarryAdder(Number* n1, Number* n2);
struct RippleCarryAdderResult RippleCarryAdder(Number* n1, Number* n2)
{	
	struct RippleCarryAdderResult adderResult;
  	
  	
  	
	return adderResult;
}

//DEFAULT METHODS
void setup()
{
	//First multiplexer config
	number1 = new Number();
	number1->Clock = 2;
	number1->Latch = 3;
	number1->Data = 4;

	pinMode(number1->Clock, OUTPUT);
	pinMode(number1->Latch, OUTPUT);
	pinMode(number1->Data, OUTPUT);
  
	Serial.begin(9600);
}
 
void loop()
{
	number1->WriteBits();
	RippleCarryAdder(number1, number1);
}