//Created by Everaldo Junior in 05/19/20. Licensed by MIT

//CUSTOM CLASS
class Number
{
	public:
		//Led multiplexer info
		int LedClock;
		int LedLatch;
		int LedData;
		
		//Port to read button data
		int ButtonRead;
		//Current button state (0 = relesed | 1 = pressed)
		int ButtonCurrentState;
		int ButtonLastState;

		//Number info Leds [b7, b6, b5, b4, b3, b2, b1, b0]
		bool Bits[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		
		//Constructor
		Number(int ledClock, int ledLatch, int ledData, int buttonRead)
		{
			LedClock = ledClock;
			LedLatch = ledLatch;
			LedData = ledData;
			
			ButtonRead = buttonRead;
			
			pinMode(LedClock, OUTPUT);
			pinMode(LedLatch, OUTPUT);
			pinMode(LedData, OUTPUT);
		}
		
		void WriteBits()
		{
			//Open data flow
			digitalWrite(LedLatch, LOW);

			for (int x = 0; x < sizeof(Bits); x++)
			{
				//Write value and pulse the clock
				digitalWrite(LedClock, LOW);
				digitalWrite(LedData, Bits[x]);
				digitalWrite(LedClock, HIGH);
			}    
			 
			//Close data flow
			digitalWrite(LedLatch, HIGH);
		}
		
		void ReadButtons()
		{
			int read = analogRead(ButtonRead);
			
			ButtonCurrentState = read < 20 ? 0 : 1;
			
			if(ButtonLastState != ButtonCurrentState)
			{
				if(read >= 400)//bit8
				{
					Bits[7] = !Bits[7];
				}
				else if(read >= 300 && read < 400)//bit7
				{
					Bits[6] = !Bits[6];
					
				}
				else if(read >= 200 && read < 300)//bit6
				{
					Bits[5] = !Bits[5];
					
				}
				else if(read >= 100 && read < 200)//bit5
				{
					Bits[4] = !Bits[4];
					
				}
				else if(read >= 80 && read < 100)//bit4
				{
					Bits[3] = !Bits[3];
					delay(10);
				}
				else if(read >= 60 && read < 80)//bit3
				{
					Bits[2] = !Bits[2];
					
				}
				else if(read >= 40 && read < 60)//bit2
				{
					Bits[1] = !Bits[1];
					
				}
				else if(read >= 20 && read < 40)//bit1
				{
					Bits[0] = !Bits[0];
					
				}
			}
			
			ButtonLastState = ButtonCurrentState;
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
	
  	for(int i = 0; i < 8; i++)
	{
		FullAdderResult result = FullAdder(n1->Bits[i], n2->Bits[i], adderResult.Carry);
		
		adderResult.Sum[i] = result.Sum;
		adderResult.Carry = result.Carry;
	}
  	
	return adderResult;
}

//DEFAULT METHODS
void setup()
{
	//First multiplexer config
	number1 = new Number(2, 3, 4, A0);
	number2 = new Number(5, 6, 7, A1);
	result = new Number(8, 9, 10, 0);
	
  	number1->Bits[0] = 1;
  	number2->Bits[7] = 1;
  
	Serial.begin(9600);
}
 
void loop()
{
  	number1->ReadButtons();
	number1->WriteBits();
	
  	number2->ReadButtons();
	number2->WriteBits();
	
	result->WriteBits();
	
	RippleCarryAdderResult adderResult = RippleCarryAdder(number1, number2);
	
	for(int i = 0; i < 8; i++)
		result->Bits[i] = adderResult.Sum[i];
}