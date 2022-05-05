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


#pragma once

//change data type from int to uint_8 and what not
#include "cstdint"
#include "random"

const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class Chip8
{
public:
	Chip8();//Constructor
	~Chip8();//Deconstructor

	uint8_t keypad[KEY_COUNT];
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT];

	void LoadROM(const char* fileName);
	void Cycle();

private:

	//typedef void () defines a pointer to function type;
	//in our case, this type is called Chip8Func
	typedef void (Chip8::* Chip8Func)();

	//PARTS OF CHIP 8
	//FIXES: added the curly brackets after each member
	uint8_t registers[REGISTER_COUNT];
	uint8_t memory[MEMORY_SIZE];
	uint16_t index;
	uint16_t pc;
	uint8_t stack[STACK_LEVELS];
	uint8_t sp;
	uint8_t delay;
	uint8_t sound;
	uint16_t opcode;

	//create function arrays that return a pointer to a function
	//instantiate all the contents of the array to point to the 
	//OP_NULL function
	//in the Chip8 Constructor we set the indexes we need to point
	//to the corresponding function
	Chip8Func table[0xF + 1]{ &Chip8::OP_NULL};
	Chip8Func table0[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func table8[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableE[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableF[0x65 + 1]{ &Chip8::OP_NULL };

	//hover over default_random_engine and you see the keyword "using"
	//so from what i understand, we are taking and using this from the
	//std class. maybe thats why we need to initialize at the beginning of constructor
	std::default_random_engine randomSeed;
	std::uniform_int_distribution<short> randomByte;

	void Table0();
	void Table8();
	void TableE();
	void TableF();
	void OP_NULL();

	void OP_00E0(); //00E0: CLS - Clear the Display
	void OP_00EE(); //00EE: RET - Return from a Subroutine
	void OP_1NNN(); //1NNN: JP addr - jump to location NNN
	void OP_2NNN(); //2NNN: CALL addr - Call subroutine at NNN
	void OP_3XKK(); //3XKK: SE Vx, byte - skip next instruction if Vx = kk
	void OP_4XKK(); //4XKK: SNE Vx, byte - Skip next instruction if Vx != kk
	void OP_5XY0(); //5XY0: SE Vx, Vy - skip next instruction if Vx = Vy
	void OP_6XKK(); //6XKK: LD Vx, Byte - Set Vx = kk
	void OP_7XKK(); //7XKK: ADD Vx, Byte - Set Vx = Vx + kk
	void OP_8XY0(); //8XY0: LD Vx, Vy - Set Vx = Vy
	void OP_8XY1(); //8XY1: OR Vx, Vy - Set Vx = Vx OR Vy
	void OP_8XY2(); //8XY2: AND Vx, Vy - Set Vx = Vx AND Vy
	void OP_8XY3(); //8XY3: XOR Vx, Vy - Set Vx = Vx XOR Vy
	void OP_8XY4(); //8XY4: ADD Vx, Vy - Set Vx = Vx + Vy, set VF = carry
	void OP_8XY5(); //8XY5: SUB Vx, Vy - Set Vx = Vx - Vy, Set VF = not Borrow
	void OP_8XY6(); //8XY6: SHR Vx - Set Vx = Vx SHR 1
	void OP_8XY7(); //8XY7: SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = Not Borrow
	void OP_8XYE(); //8XYE: SHL Vx - Set Vx = Vx SHL 1
	void OP_9XY0(); //9XY0: SNE Vx, Vy - Skip Next Instruction if Vx != Vy
	void OP_ANNN(); //ANNN: LD I, addr - Set I = NNN
	void OP_BNNN(); //BNNN: JP V0, addr - jump to location nnn + V0
	void OP_CXKK(); //CXKK: RND Vx, Byte - Set Vx = random byte AND kk
	void OP_DXYN(); //DXYN: DRW Vx, Vy, nibble - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = Collision
	void OP_EXA1(); //EXA1: SKNP Vx - Skip next instruction if key with the value of Vx is not pressed
	void OP_EX9E(); //EX9E: SKP Vx - Skip next instruction if key with the same value of Vx is pressed
	void OP_FX07(); //FX07: LD Vx, DT - Set Vx = delay timer value
	void OP_FX0A(); //FX0A: LD Vx, K - Wait for a key press, store the value of the key in Vx
	void OP_FX15(); //FX15: LD Dt, Vx - Set delay timer = Vx
	void OP_FX18(); //FX18: LD ST, Vx - Set sound timer = Vx
	void OP_FX1E(); //FX1E: Add I, Vx - Set I = I + Vx
	void OP_FX29(); //FX29: LD F, Vx - Set I = Location of sprite for digit Vx
	void OP_FX33(); //FX33: LD B, Vx - Store BCD representation of Vx in memory locations I, I+1, and I+2
	void OP_FX55(); //FX55: LD [I], Vx - Store registers V0 through Vx in memory starting in location I
	void OP_FX65(); //FX65: LD Vx, [I] - Read Registers V0 through Vx from memory starting at locatin I
};

