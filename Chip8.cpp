/***************************************************
*  Code is from Austin Morlan's Webisite           *
*  https://austinmorlan.com/posts/chip8_emulator/  *
*                                                  *     
*  Comments By: Sergio Gonzalez (ghostlySmG)       *
*                                                  *
*  Added lots of comments to hopefully make        *
*  understanding the code much easier and to be    *
*  used as reference material for future emulators *
***************************************************/



#include "Chip8.hpp"
#include "fstream" //for input and output streams
#include "random" //obviously for random number stuff
#include "chrono" //for clock stuff (date / time)

//bitwise operators for reference:
// https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit#:~:text=Toggling%20a%20bit,n%20th%20bit%20of%20number%20.


const unsigned int START_ADDRESS = 0x200;
const unsigned int FONT_START_ADDRESS = 0x50;
const unsigned int FONTSET_SIZE = 80;

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

				//this part is weird need to do more research on this
				//apparentally called an initialization list.
				//we have the member (or variable to make it easier to understand)
				//followed by (), inside of the paranthesis is what we are initialzing
//Constructor	//the member too.
Chip8::Chip8() : randomSeed(std::chrono::system_clock::now().time_since_epoch().count())
{
	//chip8 memory is reserved from addresses 0x000 to 0x1FF
	//so ROM instructions start at 0x200
	pc = START_ADDRESS;

	//add the fonts to memory
	for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
	{
		memory[FONT_START_ADDRESS + i] = fontset[i];
	}

	//this is the actual generator. it will generate a random byte from 0 to 255;
	randomByte = std::uniform_int_distribution<short>(0, 255u);

	//this table is the main table.
	//it looks at the 4 bits of the opcode (the left most bits)
	//if the first 4 bits equals 0, 8, E, or F then it will call
	//one of the Table Functions.
	//else it will call one of the opcode functions
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1NNN;
	table[0x2] = &Chip8::OP_2NNN;
	table[0x3] = &Chip8::OP_3XKK;
	table[0x4] = &Chip8::OP_4XKK;
	table[0x5] = &Chip8::OP_5XY0;
	table[0x6] = &Chip8::OP_6XKK;
	table[0x7] = &Chip8::OP_7XKK;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9XY0;
	table[0xA] = &Chip8::OP_ANNN;
	table[0xB] = &Chip8::OP_BNNN;
	table[0xC] = &Chip8::OP_CXKK;
	table[0xD] = &Chip8::OP_DXYN;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	//if first 4 bits equals 0 then check last 4 bits
	//of opcode with table0 to call opcode function
	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	//if first 4 bits equals 8 then check last 4 bits
	//of opcode with table8 to call opcode function
	table8[0x0] = &Chip8::OP_8XY0;
	table8[0x1] = &Chip8::OP_8XY1;
	table8[0x2] = &Chip8::OP_8XY2;
	table8[0x3] = &Chip8::OP_8XY3;
	table8[0x4] = &Chip8::OP_8XY4;
	table8[0x5] = &Chip8::OP_8XY5;
	table8[0x6] = &Chip8::OP_8XY6;
	table8[0x7] = &Chip8::OP_8XY7;
	table8[0xE] = &Chip8::OP_8XYE;

	//if first 4 bits equals E then check last 4 bits
	//of opcode with tableE to call opcode function
	tableE[0x1] = &Chip8::OP_EXA1;
	tableE[0xE] = &Chip8::OP_EX9E;

	//if first 4 bits equals F then check last 4 bits
	//of opcode with tableF to call opcode function
	tableF[0x07] = &Chip8::OP_FX07;
	tableF[0x0A] = &Chip8::OP_FX0A;
	tableF[0x15] = &Chip8::OP_FX15;
	tableF[0x18] = &Chip8::OP_FX18;
	tableF[0x1E] = &Chip8::OP_FX1E;
	tableF[0x29] = &Chip8::OP_FX29;
	tableF[0x33] = &Chip8::OP_FX33;
	tableF[0x55] = &Chip8::OP_FX55;
	tableF[0x65] = &Chip8::OP_FX65;
}

//Deconstructor
Chip8::~Chip8()
{
	
}

/*
Get Data from ROM file and load it to memory

Parameters: 
fileName = Name of the ROM file

Returns:
Absolutely nothing
*/
void Chip8::LoadROM(char const* fileName) 
{
	//open file stream.
	//ios::ate = start at the end of the file
	//ios::binary = the contents of the file is binary
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	//if stream is open then do the stuff
	if (file.is_open())
	{
		//since our position is at the end of the file,
		//we can use that posistion to determine the
		//size of the file

		//file.tellg returns the position of the 
		//current character in the input stream.
		//return type is std::streampos
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		//go back to the beginning of the file and fill the buffer
		//0 is the offset value relative to the 2nd argument (in this case the beginning)
		file.seekg(0, std::ios::beg);
		//contents of file go to the buffer and we specify
		//how many characters to read (which we got from file.tellg)
		file.read(buffer, size);
		file.close();

		//now that we have the contents of the ROM, its time to load
		//it to memory
		for (long i = 0; i < size; ++i) 
		{
			memory[START_ADDRESS + i] = buffer[i];
		}

		//keyword for deleting arrays from memory in c++
		delete[] buffer;
	}
}

void Chip8::Cycle()
{
	//each place in memory is only 8 bits, an opcode is 16bits
	//so we fetch a byte from memory, shift it a byte to the left
	//then get the next byte from memory and set it to the right
	//most byte of the opcode.
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	//since we already have the opcode, we can increment the program counter
	pc += 2;

	//Decode & Fetch
	//(*this) = dereferenced pointer. so returns the object
	//			instead of a pointer to the current object (this)
	//remember that table,table0,table8,tableE, and tableF are of type 
	//Chip8* and we called it Chip8Func.
	//so table[value] returns a pointer to a function.
	//so the *table[value] dereferences that pointer
	//which results in the function being called
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement the delay timer if it's been set
	if (delay > 0)
	{
		--delay;
	}

	// Decrement the sound timer if it's been set
	if (sound > 0)
	{
		--sound;
	}
}

void Chip8::Table0()
{
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8()
{
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE()
{
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF()
{
	((*this).*(tableF[opcode & 0x00FFu]))();
}

void Chip8::OP_NULL()
{

}

/* 00E0: CLS
Clear the Display */
void Chip8::OP_00E0()
{
	//sets the entire video buffer to zeroes
	memset(video, 0, sizeof(video));
}

/* 00EE: RET
Return from a subroutine */
void Chip8::OP_00EE()
{
	sp--;
	pc = stack[sp];
}

/* 1NNN: JP addr 
Jump to Location nnn*/
void Chip8::OP_1NNN()
{
	uint16_t address = opcode & 0x0FFFu;

	pc = address;
}

/* 2NNN: CALL addr
call subroutine at NNN */
void Chip8::OP_2NNN()
{
	uint16_t address = opcode & 0x0FFFu;

	stack[sp] = pc;
	sp++;
	pc = address;
}

/* 3XKK: SE Vx, Byte
Skip next instructino if X = KK */
void Chip8::OP_3XKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte)
	{
		pc += 2;
	}
}

/* 4XKK: SNE Vx, Byte
Skip next instruction if X != KK */
void Chip8::OP_4XKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = (opcode & 0x00FFu);

	if (registers[Vx] != byte) 
	{
		pc += 2;
	}
}

/* 5XY0: SE Vx, Vy
Skip next instruction if Vx = Vy*/
void Chip8::OP_5XY0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] == registers[Vy])
	{
		pc += 2;
	}
}

/* 6XKK: LD Vx, Byte
Set Vx = KK */
void Chip8::OP_6XKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = (opcode & 0x00FFu);

	registers[Vx] = byte;
}

/* 7XKK: ADD Vx, Byte
Set Vx = Vx + kk */
void Chip8::OP_7XKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = (opcode & 0x00FFu);

	registers[Vx] += byte;
}

/* 8XY0: LD Vx, Vy
Set Vx = Vy */
void Chip8::OP_8XY0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

/* 8XY1: OR Vx, Vy
Set Vx = Vx OR Vy */
void Chip8::OP_8XY1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

/* 8XY2: AND Vx, Vy
Set Vx = Vx AND Vy */
void Chip8::OP_8XY2()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

/* 8XY3: XOR Vx, Vy
Set Vx = Vx XOR Vy */
void Chip8::OP_8XY3()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}

/* 8XY4: ADD Vx, Vy
Set Vx = Vx + Vy, Set VF = carry*/
void Chip8::OP_8XY4()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255u)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	//registers[Vx] = registers[Vy];
	registers[Vx] = sum & 0x00FFu;
}

/* 8XY5: SUB Vx, Vy
Set Vx = Vx - Vy, set VF = Not Borrow*/
void Chip8::OP_8XY5()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] > registers[Vy])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] -= registers[Vy];
}

/* 8XY6: SHR Vx
Set Vx = Vx SHR 1*/
void Chip8::OP_8XY6()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF] = (registers[Vx] & 0x01u);

	registers[Vx] = registers[Vx] >> 1u;
}

/* 8XY7: SUBN Vx, Vy
Set Vx = Vy - Vx, Set VF = not borrow */
void Chip8::OP_8XY7()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

/* 8XYE - SHL Vx
Set Vx = Vx SHL 1 */
void Chip8::OP_8XYE()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] = registers[Vx] << 1u;
}

/* 9XY0: SNE Vx, Vy
Skip Next instruction if Vx != Vy */
void Chip8::OP_9XY0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

/* ANNN: LD I, addr
Set I = nnn */
void Chip8::OP_ANNN()
{
	uint16_t address = (opcode & 0x0FFFu);

	index = address;
}

/* BNNN: JP V0, addr
jump to location nnn + V0 */
void Chip8::OP_BNNN()
{
	uint16_t address = (opcode & 0x0FFFu);

	pc = registers[0] + address;
}

/* CXKK: RND Vx, Byte
Set Vx = random byte AND kk*/
void Chip8::OP_CXKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = (opcode & 0x00FFu);

	//random byte produces a value between 0x0000 and 0x00FF
	//just in case, i cast to uint8_t....just to be safe...hopefully
	uint8_t r = (uint8_t)(randomByte(randomSeed) & byte);
	registers[Vx] = r;
}

/* DXYN: DRW Vx, Vy, nibble
Display n-byte sprite starting at memory location I
at (Vx, Vy), set VF = Collision*/
void Chip8::OP_DXYN()
{
	//get data from opcode
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = (opcode & 0x000Fu);

	//set VF = 0
	registers[0xF] = 0;

	//set x and y positions.
	//the mod keeps the x and ypos withing the video size
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	for (unsigned int row = 0; row < height; ++row)
	{

		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);

			//screenPixel is a pointer to the pixel
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel (XOR Toggles)
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

/* EX9E: SKP Vx
Skip next instruction if key with the value of 
Vx is pressed */
void Chip8::OP_EX9E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (keypad[key])
	{
		pc += 2;
	}
}

/* EXA1: SKNP Vx
Skip next instruction if key with the value of
Vx is not pressed */
void Chip8::OP_EXA1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (!keypad[key])
	{
		pc += 2;
	}
}

/* FX07: LD Vx, Dt
Set Vx = delay timer value*/
void Chip8::OP_FX07()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	
	registers[Vx] = delay;
}

/* FX0A: LD Vx, K
Wait for a key press, store the value of the
key in Vx*/
void Chip8::OP_FX0A()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		//we incrememnt pc + 2 to the next instruction
		//in Cycle(). so decrementing by 2 will cause the
		//same instruction to be ran over and over again
		pc -= 2;
	}
}

/* FX15: LD Dt, Vx
Set delay timer = Vx */
void Chip8::OP_FX15()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delay = registers[Vx];
}

/* FX18: LD ST, Vx
Set sound timer = Vx */
void Chip8::OP_FX18()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	sound = registers[Vx];
}

/* FX1E: ADD I, Vx
Set I = I + Vx */
void Chip8::OP_FX1E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index += registers[Vx];
}

/* FX29: LD F, Vx
Set I = location of sprite for digit Vx */
void Chip8::OP_FX29()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	//we multiply by 5 since each font has 5 bytes
	index = FONT_START_ADDRESS + (5 * digit);
}

/* FX33: LD B, Vx
Store BCD representation of Vx in memory
locatins I, I+1, and I+2 
The interpreter takes the decimal value of Vx, 
and places the hundreds digit in memory at location
in I, the tens digit at location I+1, and the ones 
digit at location I+2.*/
void Chip8::OP_FX33()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	// Ones-place
	memory[index + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index] = value % 10;
}

/* FX55: LD [I], Vx
Store registers V0 through Vx in memory
starting at location I */
void Chip8::OP_FX55()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[i + 1] = registers[i];
	}
}

/* FX65: LD Vx, [I]
Read registers V0 through Vx from memory
starting at location I */
void Chip8::OP_FX65()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}
}
