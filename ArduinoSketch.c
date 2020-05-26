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
		Number()
		{
		}
		
		Number(int ledClock, int ledLatch, int ledData, int buttonRead)
		{
			LedClock = ledClock;
			LedLatch = ledLatch;
			LedData = ledData;
			
			ButtonRead = buttonRead;
			
			pinMode(LedClock, OUTPUT);
			pinMode(LedLatch, OUTPUT);
			pinMode(LedData, OUTPUT);
			pinMode(ButtonRead, INPUT);
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
					Bits[7] = !Bits[7];
				else if(read >= 300 && read < 400)//bit7
					Bits[6] = !Bits[6];
				else if(read >= 200 && read < 300)//bit6
					Bits[5] = !Bits[5];
				else if(read >= 100 && read < 200)//bit5
					Bits[4] = !Bits[4];
				else if(read >= 80 && read < 100)//bit4
					Bits[3] = !Bits[3];
				else if(read >= 60 && read < 80)//bit3
					Bits[2] = !Bits[2];
				else if(read >= 40 && read < 60)//bit2
					Bits[1] = !Bits[1];
				else if(read >= 20 && read < 40)//bit1
					Bits[0] = !Bits[0];
					
				//Write all bits
				WriteBits();
			}
			
			ButtonLastState = ButtonCurrentState;
		}
};

struct FullAdderResult
{
	bool Sum = 0;
	bool Carry = 0;
};

struct RippleCarryAdderResult
{
	bool Sum[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool Carry = 0;
};

//VARIABLES
Number* number1;
Number* number2;
Number* result;

int actionButtonsPort = A2;
int signalLedPort = 11;

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

//Turn on all result leds and torn off
void ShowOverflow()
{
	for(int x = 0; x < 4; x++)
	{
		//Write on result number
		for(int i = 0; i < 8; i++)
			result->Bits[i] = 1;
		
		result->WriteBits();
		delay(300);
		
		for(int i = 0; i < 8; i++)
			result->Bits[i] = 0;

		result->WriteBits();
		delay(300);
	}
}

//DEFAULT METHODS
void setup()
{
	pinMode(actionButtonsPort, INPUT);
	pinMode(signalLedPort, OUTPUT);
  
	//First multiplexer config
	number1 = new Number(2, 3, 4, A0);
	number2 = new Number(5, 6, 7, A1);
	result = new Number(8, 9, 10, -1);
  
	Serial.begin(9600);
}
 
void loop()
{
  	number1->ReadButtons();
  	number2->ReadButtons();
		
	int actionButtonRead = analogRead(actionButtonsPort);
	
	//Sum action
	if(actionButtonRead >= 500)
	{
		//Execute the sum
		RippleCarryAdderResult adderResult = RippleCarryAdder(number1, number2);
	
		//Overflow
		if(adderResult.Carry)
		{
			//Show Overflow warning
			ShowOverflow();
		}
		else
		{
			//Write on result number
			for(int i = 0; i < 8; i++)
				result->Bits[i] = adderResult.Sum[i];
			
			//Write on leds
			result->WriteBits();
		}
	}
	//Subtraction action
	else if(actionButtonRead >= 300 && actionButtonRead < 400)
	{
        //Transform bin number to twos complement
		Number* negativeNumber = new Number();
        for(int i = 0; i < 8; i++)
        {
            negativeNumber->Bits[i] = number2->Bits[i];
            if(number2->Bits[i])
            {
                for(int j = i + 1; j < 8; j++)
                    negativeNumber->Bits[j] = !number2->Bits[j];
                break;
            }
        }

        //Execute the sum
		RippleCarryAdderResult adderResult = RippleCarryAdder(number1, negativeNumber);
		
		//Invert result if carry was 0
		if(!adderResult.Carry)
		{
			digitalWrite(signalLedPort, HIGH);
			for(int i = 0; i < 8; i++)
			{
				if(adderResult.Sum[i])
				{
					for(int j = i + 1; j < 8; j++)
						adderResult.Sum[j] = !adderResult.Sum[j];
					break;
				}
			}
		}
		
		//Write on result number
		for(int i = 0; i < 8; i++)
			result->Bits[i] = adderResult.Sum[i];
		
		//Write on leds
		result->WriteBits();

		//Free arduino memory
		free(negativeNumber);
	}
	//Multiplication action
	else if(actionButtonRead >= 200 && actionButtonRead < 300)
	{
		//Create a new number to save the multiplication
		Number* multNumber = new Number();
		
		for(int i = 0; i < 8; i++)
		{
			Number* tempNumber = new Number();
			for(int j = 0; j < 8; j++)
			{
				if(i + j > 8 && (number2->Bits[i] * number1->Bits[j]) != 0)
				{
					//Display overflow error
					ShowOverflow();
					
					//Free the memory
					free(tempNumber);
					free(multNumber);
					return;
				}
				else if(i + j < 8)
				{
					//Execute bit to bit multiplication
					tempNumber->Bits[j + i] = number2->Bits[i] * number1->Bits[j];	
				}
			}
			
			//Execute the sum
			RippleCarryAdderResult adderResult = RippleCarryAdder(multNumber, tempNumber);
			
			if(adderResult.Carry)
			{
				//Display overflow error
				ShowOverflow();
				
				//Free the memory
				free(tempNumber);
				free(multNumber);
				return;
			}
			else
			{
				//Save the sum
				for(int x = 0; x < 8; x++)
					multNumber->Bits[x] = adderResult.Sum[x];
				
				//Free the memory
				free(tempNumber);
			}
		}
		
		//Write on result number
		for(int i = 0; i < 8; i++)
			result->Bits[i] = multNumber->Bits[i];
		
		//Write on leds
		result->WriteBits();
		
		//Free the memory
		free(multNumber);
	}
	//Division action (division using successive sum)
	else if(actionButtonRead >= 100 && actionButtonRead < 200)
	{
		bool greater = false;
		Number* currentMultiplier = new Number();
		Number* numberOne = new Number();
		numberOne->Bits[0] = 1;
		
		while(!greater)
		{
			//Create a new number to save the multiplication
			Number* multNumber = new Number();
			
			for(int i = 0; i < 8; i++)
			{
				Number* tempNumber = new Number();
				for(int j = 0; j < 8; j++)
				{
					if(i + j > 8 && (currentMultiplier->Bits[i] * number2->Bits[j]) != 0)
					{
						//Display overflow error
						ShowOverflow();
						
						//Free the memory
						free(tempNumber);
						free(multNumber);
						free(currentMultiplier);
						free(numberOne);
						return;
					}
					else if(i + j < 8)
					{
						//Execute bit to bit multiplication
						tempNumber->Bits[j + i] = currentMultiplier->Bits[i] * number2->Bits[j];	
					}
				}
				
				//Execute the sum
				RippleCarryAdderResult adderResult = RippleCarryAdder(multNumber, tempNumber);
				
				if(adderResult.Carry)
				{
					//Display overflow error
					ShowOverflow();
					
					//Free the memory
					free(tempNumber);
					free(multNumber);
					free(currentMultiplier);
					free(numberOne);
					return;
				}
				else
				{
					//Save the sum
					for(int x = 0; x < 8; x++)
						multNumber->Bits[x] = adderResult.Sum[x];
					
					//Free the memory
					free(tempNumber);
				}
			}
			
			//Write on result number
			if(number1->Bits[0] == multNumber->Bits[0]
			&& number1->Bits[1] == multNumber->Bits[1]
			&& number1->Bits[2] == multNumber->Bits[2]
			&& number1->Bits[3] == multNumber->Bits[3]
			&& number1->Bits[4] == multNumber->Bits[4]
			&& number1->Bits[5] == multNumber->Bits[5]
			&& number1->Bits[6] == multNumber->Bits[6]
			&& number1->Bits[7] == multNumber->Bits[7])
			{
				greater = true;
				
				//Write on result number
				for(int i = 0; i < 8; i++)
					result->Bits[i] = currentMultiplier->Bits[i];
				
				//Write on leds
				result->WriteBits();
			}
			else
			{
				//Execute the sum
				RippleCarryAdderResult adderResult = RippleCarryAdder(currentMultiplier, numberOne);
					
				if(adderResult.Carry)
				{
					//Display overflow error
					ShowOverflow();
					
					//Free the memory
					free(multNumber);
					free(currentMultiplier);
					free(numberOne);
					return;
				}
				else
				{
					//Save the sum
					for(int x = 0; x < 8; x++)
						currentMultiplier->Bits[x] = adderResult.Sum[x];
				}
			}
			
			//Free the memory
			free(multNumber);
		}
		
		//Free the memory
		free(currentMultiplier);
		free(numberOne);
	}
	//Reset action
	else if(actionButtonRead > 0 && actionButtonRead < 100)
	{
		//Reset all numbers
		for(int i = 0; i < 8; i++)
		{
			number1->Bits[i] = 0;
			number2->Bits[i] = 0;
			result->Bits[i] = 0;
		}
		
		number1->WriteBits();
		number2->WriteBits();
		result->WriteBits();
		digitalWrite(signalLedPort, LOW);
	}
}