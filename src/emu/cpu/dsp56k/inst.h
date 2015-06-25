#ifndef __DSP56K_INSTRUCTION_H__
#define __DSP56K_INSTRUCTION_H__

#include <string>

#include "opcode.h"
#include "tables.h"

#include "dsp56k.h"
#include "dsp56pcu.h"

//
// An Instruction is the base class all regular ops inherit from.
//
namespace DSP56K
{

class Opcode;

class Instruction
{
public:
	Instruction(const Opcode* oco) : m_valid(false),
									 m_oco(oco),
									 m_sizeIncrement(0),
									 m_opcode(""),
									 m_source(""),
									 m_destination("") { }
	virtual ~Instruction() {}

	virtual bool decode(const UINT16 word0, const UINT16 word1) = 0;
	virtual void disassemble(std::string& retString) const = 0;
	virtual void evaluate(dsp56k_core* cpustate) = 0;

	virtual size_t size() const = 0;
    virtual size_t evalSize() const { return size(); }
	virtual size_t accumulatorBitsModified() const = 0;   // Potentially make this always return ALL (like flags)
	virtual size_t flags() const { return 0; }

	static Instruction* decodeInstruction(const Opcode* opc,
										  const UINT16 word0,
										  const UINT16 word1,
										  bool shifted=false);

	const bool valid() const { return m_valid; }

	const std::string& opcode() const { return m_opcode; }
	const std::string& source() const { return m_source; }
	const std::string& destination() const { return m_destination; }

	size_t sizeIncrement() const { return m_sizeIncrement; }

protected:
	bool m_valid;
	const Opcode* m_oco;
	size_t m_sizeIncrement;

	// Parameters nearly everyone has
	std::string m_opcode;
	std::string m_source;
	std::string m_destination;
};


////////////////////////////////////////////////////////////////////////////////
//  OPS                             ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// ABS : .... .... 0111 F001 : A-18 ////////////////////////////////////////////
class Abs: public Instruction
{
public:
	Abs(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "abs";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ADC : 0001 0101 0000 F01J : A-20 ////////////////////////////////////////////
class Adc: public Instruction
{
public:
	Adc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_JF_table(BITSn(word0,0x0001), BITSn(word0,0x0008),
						m_source, m_destination);
		m_opcode = "adc";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ADD : .... .... 0000 FJJJ : A-22 ////////////////////////////////////////////
class Add: public Instruction
{
public:
	Add(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_JJJF_table(BITSn(word0,0x07), BITSn(word0,0x08),
						  m_source, m_destination);
		m_opcode = "add";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ??? Odd that i should put the 011m mKKK everywhere ???  TODO
// ADD : 011m mKKK 0rru Fuuu : A-22 ////////////////////////////////////////////
class Add_2: public Instruction
{
public:
	Add_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_arg = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_uuuuF_table(BITSn(word0,0x17), BITSn(word0,0x08),
						   m_opcode, m_source, m_destination);
		// TODO: m_opcode = "add";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_arg;  // TODO: get rid of this Add|Sub thing.
};

// AND : .... .... 0110 F1JJ : A-24 ////////////////////////////////////////////
class And: public Instruction
{
public:
	And(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_JJF_table(BITSn(word0,0x03),BITSn(word0,0x08),
						 m_source, m_destination);
		m_opcode = "and";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// ANDI : 0001 1EE0 iiii iiii : A-26 ///////////////////////////////////////////
class Andi: public Instruction
{
public:
	Andi(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = BITSn(word0,0x00ff);
		decode_EE_table(BITSn(word0,0x0600), m_destination);

		m_opcode = "andi";
		// NEW // sprintf(opcode_str, "and(i)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, "#$%x,%s", m_immediate, m_destination.c_str());
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	UINT8 m_immediate;
};

// ASL : .... .... 0011 F001 : A-28 ////////////////////////////////////////////
class Asl: public Instruction
{
public:
	Asl(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "asl";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ASL4 : 0001 0101 0011 F001 : A-30 ///////////////////////////////////////////
class Asl4: public Instruction
{
public:
	Asl4(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x0008), m_destination);
		m_opcode = "asl4";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ASR : .... .... 0011 F000 : A-32 ////////////////////////////////////////////
class Asr: public Instruction
{
public:
	Asr(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "asr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ASR4 : 0001 0101 0011 F000 : A-34 ///////////////////////////////////////////
class Asr4: public Instruction
{
public:
	Asr4(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x0008), m_destination);
		m_opcode = "asr4";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ASR16 : 0001 0101 0111 F000 : A-36 //////////////////////////////////////////
class Asr16: public Instruction
{
public:
	Asr16(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x0008), m_destination);
		m_opcode = "asr16";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

/* BFCHG  : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
/* BFCHG  : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
/* BFCHG  : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
/* BFCLR  : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
/* BFCLR  : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
/* BFSET  : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
/* BFSET  : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
class BfInstruction: public Instruction
{
public:
	BfInstruction(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		/* Decode the common parts */
		UINT16 iVal = 0x0000;
		iVal = BITSn(word1,0x00ff);

		int upperMiddleLower = -1;
		upperMiddleLower = decode_BBB_table(BITSn(word1,0xe000));

		switch(upperMiddleLower)
		{
			case BBB_UPPER:  iVal <<= 8; break;
			case BBB_MIDDLE: iVal <<= 4; break;
			case BBB_LOWER:  iVal <<= 0; break;

			case BBB_INVALID: return false; break;
		}

		switch(BITSn(word0,0x00e0))
		{
			case 0x6: case 0x7: case 0x2: case 0x3:
				assemble_D_from_P_table(BITSn(word0,0x0020), BITSn(word0,0x001f), m_destination);
				break;
			case 0x5: case 0x1:
				INT8 rNum;
				char temp[32];
				decode_RR_table(BITSn(word0,0x0003), rNum);
				sprintf(temp, "X:(R%d)", rNum);
				m_destination = temp;
				break;
			case 0x4: case 0x0:
				decode_DDDDD_table(BITSn(word0,0x001f), m_destination);
				break;
		}

		if (m_destination == "!!")
			return false;

		char temp[32];
		sprintf(temp, "#$%x", iVal);
		// NEW // sprintf(temp, "#$%04x", iVal);
		m_source = temp;

		switch(BITSn(word1,0x1f00))
		{
			case 0x12: m_opcode = "bfchg";  break;
			case 0x04: m_opcode = "bfclr";  break;
			case 0x18: m_opcode = "bfset";  break;
			case 0x10: m_opcode = "bftsth"; break;
			case 0x00: m_opcode = "bftstl"; break;
		}
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 ////////////////////////
class Bcc: public Instruction
{
public:
	Bcc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = (INT16)word1;

		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "b" + M;
		// NEW // sprintf(opcode_str, "b.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, ">*+$%x", 2 + m_immediate);
		// NEW // sprintf(temp, "$%04x (%d)", pc + 2 + (INT16)word1, (INT16)word1);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	INT16 m_immediate;
};

// Bcc : 0010 11cc ccee eeee : A-48 ////////////////////////////////////////////
class Bcc_2: public Instruction
{
public:
	Bcc_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		std::string M;
		decode_cccc_table(BITSn(word0,0x3c0), M);
		m_immediate = get_6_bit_signed_value(BITSn(word0,0x003f));
		m_opcode = "b" + M;
		// NEW // sprintf(opcode_str, "b.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		if (m_immediate >= 0) sprintf(temp, "<*+$%x", m_immediate + 1);
		else                  sprintf(temp, "<*-$%x", 1 - m_immediate - 2);
		// NEW // sprintf(temp, "$%04x (%d)", pc + 1 + relativeInt, relativeInt);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	INT8 m_immediate;
};

// Bcc : 0000 0111 RR10 cccc : A-48 ////////////////////////////////////////////
class Bcc_3: public Instruction
{
public:
	Bcc_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x00c0), rNum);
		sprintf(temp, "R%d", rNum);
		m_destination = temp;

		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "b" + M;
		// NEW // sprintf(opcode_str, "b.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 ////////////////////////
class Bra: public Instruction
{
public:
	Bra(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = (INT16)word1;

		m_opcode = "bra";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, ">*+$%x", 2 + m_immediate);
		// NEW // sprintf(temp, "$%04x (%d)", pc + 2 + word1, (INT16)word1);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	INT16 m_immediate;
};

// BRA : 0000 1011 aaaa aaaa : A-50 ////////////////////////////////////////////
class Bra_2: public Instruction
{
public:
	Bra_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = (INT8)BITSn(word0,0x00ff);
		m_opcode = "bra";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		if (m_immediate >= 0) sprintf(temp, "<*+$%x", 1 + m_immediate);
		else                  sprintf(temp, "<*-$%x", 1 - m_immediate - 2);
		// NEW // sprintf(temp, "$%04x (%d)", pc + 1 + iVal, iVal);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	INT8 m_immediate;
};

// BRA : 0000 0001 0010 11RR : A-50 ////////////////////////////////////////////
class Bra_3: public Instruction
{
public:
	Bra_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x0003), rNum);
		sprintf(temp, "R%d", rNum);
		m_destination = temp;

		m_opcode = "bra";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// BRKcc : 0000 0001 0001 cccc : A-52 //////////////////////////////////////////
class Brkcc: public Instruction
{
public:
	Brkcc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "brk" + M;
		// NEW // sprintf(opcode_str, "brk.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 ///////////////////////
class Bscc: public Instruction
{
public:
	Bscc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = (INT16)word1;

		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "bs" + M;
		// NEW // sprintf(opcode_str, "bs.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		if (m_immediate >= 0) sprintf(temp, ">*+$%x", 2 + m_immediate);
		else                  sprintf(temp, ">*-$%x", 1 - m_immediate - 1 - 2);
		//sprintf(temp, ">*+$%x", 2 + m_immediate);
		// NEW // sprintf(temp, "$%04x (%d)", pc + 2 + (INT16)word1, (INT16)word1);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OVER; }

private:
	INT16 m_immediate;
};

// BScc : 0000 0111 RR00 cccc : A-54 ///////////////////////////////////////////
class Bscc_2: public Instruction
{
public:
	Bscc_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x00c0), rNum);
		sprintf(temp, "R%d", rNum);

		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_destination = temp;

		m_opcode = "bs" + M;
		// NEW // sprintf(opcode_str, "bs.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OVER; }
};

// BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 ////////////////////////
class Bsr: public Instruction
{
public:
	Bsr(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = (INT16)word1;

		m_opcode = "bsr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		if (m_immediate >= 0) sprintf(temp, ">*+$%x", 2 + m_immediate);
		else                  sprintf(temp, ">*-$%x", 1 - m_immediate - 1 - 2);
		// NEW // sprintf(temp, "$%04x (%d)", pc + 2 + (INT16)word1, (INT16)word1);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OVER; }

private:
	INT16 m_immediate;
};

// BSR : 0000 0001 0010 10RR : A-56 ////////////////////////////////////////////
class Bsr_2: public Instruction
{
public:
	Bsr_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x0003), rNum);
		sprintf(temp, "R%d", rNum);
		m_destination = temp;

		m_opcode = "bsr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OVER; }
};

// CHKAAU : 0000 0000 0000 0100 : A-58 /////////////////////////////////////////
class Chkaau: public Instruction
{
public:
	Chkaau(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "chkaau";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// CLR : .... .... 0000 F001 : A-60 ////////////////////////////////////////////
class Clr: public Instruction
{
public:
	Clr(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "clr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// CLR24 : .... .... 0101 F001 : A-62 //////////////////////////////////////////
class Clr24: public Instruction
{
public:
	Clr24(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "clr24";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// CMP : .... .... 0101 FJJJ : A-64 ////////////////////////////////////////////
class Cmp: public Instruction
{
public:
	Cmp(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		/* Note: This is a JJJF limited in the docs, but other opcodes sneak
                 in before cmp, so the same decode function can be used. */
		decode_JJJF_table(BITSn(word0,0x07), BITSn(word0,0x08),
						  m_source, m_destination);
		m_opcode = "cmp";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_NONE; }
};

// CMPM : .... .... 0111 FJJJ : A-66 ///////////////////////////////////////////
class Cmpm: public Instruction
{
public:
	Cmpm(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		/* Note: This is a JJJF limited in the docs, but other opcodes sneak
                 in before cmp, so the same decode function can be used. */
		decode_JJJF_table(BITSn(word0,0x07), BITSn(word0,0x08),
						  m_source, m_destination);
		m_opcode = "cmpm";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_NONE; }
};

// DEBUG : 0000 0000 0000 0001 : A-68 //////////////////////////////////////////
class Debug: public Instruction
{
public:
	Debug(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "debug";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// DEBUGcc : 0000 0000 0101 cccc : A-70 ////////////////////////////////////////
class Debugcc: public Instruction
{
public:
	Debugcc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "debug" + M;
		// NEW // sprintf(opcode_str, "debug.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// DEC : .... .... 0110 F010 : A-72 ////////////////////////////////////////////
class Dec: public Instruction
{
public:
	Dec(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "dec";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// DEC24 : .... .... 0110 F011 : A-74 //////////////////////////////////////////
class Dec24: public Instruction
{
public:
	Dec24(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "dec24";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// DIV : 0001 0101 0--0 F1DD : A-76 ////////////////////////////////////////////
class Div: public Instruction
{
public:
	Div(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_DDF_table(BITSn(word0,0x0003), BITSn(word0,0x0008),
						 m_source, m_destination);
		m_opcode = "div";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// DMAC : 0001 0101 10s1 FsQQ : A-80 ///////////////////////////////////////////
class Dmac: public Instruction
{
public:
	Dmac(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQF_special_table(BITSn(word0,0x0003), BITSn(word0,0x0008),
								 m_source, m_source2, m_destination);

		std::string a;
		decode_ss_table(BITSn(word0,0x0024), a);
		if (a == "!!") return false;

		m_opcode = "dmac" + a;
		// NEW // sprintf(opcode_str, "dmac(%s)", A);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 /////////////////////////
class Do: public Instruction
{
public:
	Do(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = word1;

		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x0003), rNum);
		sprintf(temp, "X:(R%d)", rNum);
		m_source = temp;

		m_opcode = "do";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, "*+$%x", 2 + m_immediate);
		// NEW // sprintf(temp, "X:(R%d),$%02x", Rnum, pc + 2 + word1);
		retString = m_opcode + " " + m_source + "," + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	UINT16 m_immediate;
};

// DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 /////////////////////////
class Do_2: public Instruction
{
public:
	Do_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_displacement = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = BITSn(word0,0x00ff);
		m_displacement = word1;

		m_opcode = "do";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, "#<$%x,*+$%x", m_immediate, 2 + m_displacement);
		// NEW // sprintf(temp, "#$%02x,$%04x", BITSn(word0,0x00ff), pc + 2 + word1);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	UINT8 m_immediate;
	UINT16 m_displacement;
};

// DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 /////////////////////////
class Do_3: public Instruction
{
public:
	Do_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_displacement = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_displacement = word1;

		decode_DDDDD_table(BITSn(word0,0x001f), m_source);
		if (m_source == "SSH") return false; // NEW //
		if (m_source == "!!") return false;  // NEW //
		m_opcode = "do";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, "*+$%x", 2 + m_displacement);
		// NEW // sprintf(temp, "%s,$%04x", S1, pc + 2 + word1);
		retString = m_opcode + " " + m_source + "," + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	UINT16 m_displacement;
};

// DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 /////////////////
class DoForever: public Instruction
{
public:
	DoForever(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_displacement = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_displacement = word1;
		m_opcode = "do forever";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, "*+$%x", m_displacement + 2);
		// NEW // sprintf(temp, "*+$%x", pc + word1);
		// NEW // sprintf(temp, "$%04x", pc + 2 + word1);
		retString = m_opcode + ", " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	UINT16 m_displacement;
};

// ENDDO : 0000 0000 0000 1001 : A-92 //////////////////////////////////////////
class Enddo: public Instruction
{
public:
	Enddo(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "enddo";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// EOR : .... .... 0011 F1JJ : A-94 ////////////////////////////////////////////
class Eor: public Instruction
{
public:
	Eor(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_JJF_table(BITSn(word0,0x03),BITSn(word0,0x08),
						 m_source, m_destination);
		m_opcode = "eor";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// EXT : 0001 0101 0101 F010 : A-96 ////////////////////////////////////////////
class Ext: public Instruction
{
public:
	Ext(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x0008), m_destination);
		m_opcode = "ext";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ILLEGAL : 0000 0000 0000 1111 : A-98 ////////////////////////////////////////
class Illegal: public Instruction
{
public:
	Illegal(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "illegal";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// IMAC : 0001 0101 1010 FQQQ : A-100 //////////////////////////////////////////
class Imac: public Instruction
{
public:
	Imac(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQQF_table(BITSn(word0,0x0007), BITSn(word0,0x0008),
						  m_source, m_source2, m_destination);
		m_opcode = "imac";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// IMPY : 0001 0101 1000 FQQQ : A-102 //////////////////////////////////////////
class Impy: public Instruction
{
public:
	Impy(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQQF_table(BITSn(word0,0x0007), BITSn(word0,0x0008),
						  m_source, m_source2, m_destination);
		m_opcode = "impy";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// INC : .... .... 0010 F010 : A-104 ///////////////////////////////////////////
class Inc: public Instruction
{
public:
	Inc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "inc";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// INC24 : .... .... 0010 F011 : A-106 /////////////////////////////////////////
class Inc24: public Instruction
{
public:
	Inc24(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "inc24";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 ///////////////////////
class Jcc: public Instruction
{
public:
	Jcc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_displacement = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_displacement = word1;

		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "j" + M;
		// NEW // sprintf(opcode_str, "j.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, ">$%x", m_displacement);
		// NEW // sprintf(temp, "$%04x", word1);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	UINT16 m_displacement;
};

// Jcc : 0000 0110 RR10 cccc : A-108 ///////////////////////////////////////////
class Jcc_2: public Instruction
{
public:
	Jcc_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x00c0), rNum);
		sprintf(temp, "R%d", rNum);
		m_destination = temp;

		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "j" + M;
		// NEW // sprintf(opcode_str, "j.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 ///////////////////////
class Jmp: public Instruction
{
public:
	Jmp(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_displacement = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_displacement = word1;

		m_opcode = "jmp";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, ">$%x", m_displacement);
		// NEW // sprintf(temp, "$%04x", word1);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	UINT16 m_displacement;
};

// JMP : 0000 0001 0010 01RR : A-110 ///////////////////////////////////////////
class Jmp_2: public Instruction
{
public:
	Jmp_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x0003), rNum);
		sprintf(temp, "R%d", rNum);
		m_destination = temp;

		m_opcode = "jmp";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate)
	{
		// Get value from source

        // Set the PC
		PC = 0;

	    /* S L E U N Z V C */
	    /* - - - - - - - - */
	}
	size_t size() const { return 1; }
    size_t evalSize() const { return 0; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 //////////////////////
class Jscc: public Instruction
{
public:
	Jscc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_displacement = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_displacement = word1;

		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "js" + M;
		// NEW // sprintf(opcode_str, "js.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, ">$%x", m_displacement);
		// NEW // sprintf(temp, "$%04x", word1);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OVER; }

private:
	UINT16 m_displacement;
};

// JScc : 0000 0110 RR00 cccc : A-112 //////////////////////////////////////////
class Jscc_2: public Instruction
{
public:
	Jscc_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x00c0), rNum);
		sprintf(temp, "R%d", rNum);
		m_destination = temp;

		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "js" + M;
		// NEW // sprintf(opcode_str, "js.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OVER; }
};

// JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 ///////////////////////
class Jsr: public Instruction
{
public:
	Jsr(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_displacement = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_displacement = word1;

		m_opcode = "jsr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, ">$%x", m_displacement);
		// NEW // sprintf(temp, "$%04x", word1);
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OVER; }

private:
	UINT16 m_displacement;
};

// JSR : 0000 1010 AAAA AAAA : A-114 ///////////////////////////////////////////
class Jsr_2: public Instruction
{
public:
	Jsr_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_bAddr = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_bAddr = BITSn(word0,0x00ff);

		m_opcode = "jsr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, "<$%x", m_bAddr);
		// NEW // sprintf(temp, "#$%02x", BITSn(word0,0x00ff));
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OVER; }

private:
	UINT8 m_bAddr;
};

// JSR : 0000 0001 0010 00RR : A-114 ///////////////////////////////////////////
class Jsr_3: public Instruction
{
public:
	Jsr_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x0003), rNum);
		sprintf(temp, "R%d", rNum);
		m_destination = temp;

		m_opcode = "jsr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OVER; }
};

// LEA : 0000 0001 11TT MMRR : A-116 ///////////////////////////////////////////
class Lea: public Instruction
{
public:
	Lea(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		if ((word0 & 0x000c) == 0) return false;  // NEW TODO //

		char temp[32];

		INT8 tNum;
		decode_TT_table(BITSn(word0,0x0030), tNum);
		sprintf(temp, "R%d", tNum);
		m_destination = temp;

		INT8 rNum;
		std::string ea;
		decode_RR_table(BITSn(word0,0x0003), rNum);
		assemble_ea_from_MM_table(BITSn(word0,0x000c), rNum, ea);
		m_source = ea;

		m_opcode = "lea";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// LEA : 0000 0001 10NN MMRR : A-116 ///////////////////////////////////////////
class Lea_2: public Instruction
{
public:
	Lea_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		if ((word0 & 0x000c) == 0) return false;  // NEW TODO //

		char temp[32];

		INT8 nNum;
		decode_NN_table(BITSn(word0,0x0030), nNum);
		sprintf(temp, "N%d", nNum);
		m_destination = temp;

		INT8 rNum;
		decode_RR_table(BITSn(word0,0x0003), rNum);

		std::string ea;
		assemble_ea_from_MM_table(BITSn(word0,0x000c), rNum, ea);
		m_source = ea;

		m_opcode = "lea";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// LSL : .... .... 0011 F011 : A-118 ///////////////////////////////////////////
class Lsl: public Instruction
{
public:
	Lsl(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "lsl";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// LSR : .... .... 0011 F010 : A-120 ///////////////////////////////////////////
class Lsr: public Instruction
{
public:
	Lsr(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);
		m_opcode = "lsr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// MAC : .... .... 1k10 FQQQ : A-122 ///////////////////////////////////////////
class Mac: public Instruction
{
public:
	Mac(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQQF_table(BITSn(word0,0x07), BITSn(word0,0x08),
						  m_source, m_source2, m_destination);

		std::string sign;
		decode_kSign_table(BITSn(word0,0x40), sign);
		if (sign == "-")
			m_source = sign + m_source; // TODO: Probably silly for Instruction

		m_opcode = "mac";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MAC : 011m mKKK 1xx0 F1QQ : A-122 ///////////////////////////////////////////
class Mac_2: public Instruction
{
public:
	Mac_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQF_table(BITSn(word0,0x03), BITSn(word0,0x08),
						 m_source, m_source2, m_destination);

		m_opcode = "mac";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MAC : 0001 0111 RRDD FQQQ : A-122 ///////////////////////////////////////////
class Mac_3: public Instruction
{
public:
	Mac_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQQF_table(BITSn(word0,0x0007), BITSn(word0,0x0008),
						  m_source, m_source2, m_destination);
		m_opcode = "mac";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MACR : .... .... 1k11 FQQQ : A-124 //////////////////////////////////////////
class Macr: public Instruction
{
public:
	Macr(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQQF_table(BITSn(word0,0x07), BITSn(word0,0x08),
						  m_source, m_source2, m_destination);

		std::string sign;
		decode_kSign_table(BITSn(word0,0x40), sign);
		if (sign == "-")
			m_source = sign + m_source; // TODO: Probably silly for Instruction

		m_opcode = "macr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MACR : 011m mKKK 1--1 F1QQ : A-124 //////////////////////////////////////////
class Macr_2: public Instruction
{
public:
	Macr_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQF_table(BITSn(word0,0x03), BITSn(word0,0x08),
						 m_source, m_source2, m_destination);
		m_opcode = "macr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 ////////////////////////////////////
class Macsuuu: public Instruction
{
public:
	Macsuuu(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		// Special QQF
		decode_QQF_special_table(BITSn(word0,0x0003), BITSn(word0,0x0008),
								 m_source, m_source2, m_destination);

		std::string a;
		decode_s_table(BITSn(word0,0x0004), a);
		m_opcode = "mac" + a;
		// NEW // sprintf(opcode_str, "mac(%s)", A);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MOVE : .... .... 0001 0001 : A-128 //////////////////////////////////////////
class Move: public Instruction
{
public:
	Move(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_isNop = false;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		// Equivalent to a NOP (+ parallel move)

		// This insures the debugger matches the reference disassembler
		// for the undocumented .... .... 0001 1001 Instruction.
		if(BITSn(word0, 0x000f) == 0x0001)
			m_destination = "A";
		else
			m_destination = "B";

		// Hack to match reference disassembler
		UINT8 BITSn = (word0 & 0xff00) >> 8;
		if (BITSn == 0x4a || BITSn == 0x4b)
			m_isNop = true;

		m_opcode = "move";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		if (m_isNop)
			retString = "nop";
		else
			retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_NONE; }

private:
	bool m_isNop;
};

// MOVE : 011m mKKK 0rr1 0000 : A-128 //////////////////////////////////////////
class Move_2: public Instruction
{
public:
	Move_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		// Amounts to a nop with two parallel moves.
		// This insures the debugger matches the reference disassembler
		if((word0 & 0x0008) == 0x0008)
			m_destination = "B";
		else
			m_destination = "A";

		m_opcode = "move";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 //////////////////////
class Move_3: public Instruction
{
public:
	Move_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_b = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_b = BITSn(word0,0x00ff);

		std::string SD;
		decode_HHH_table(BITSn(word1,0x0e00), SD);
		assemble_reg_from_W_table(BITSn(word1,0x0100), 'X', SD, m_b,
								  m_source, m_destination);

		m_opcode = "move";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	INT8 m_b;
};

// MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 ///////////////////////////////////////
class Movec: public Instruction
{
public:
	Movec(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		std::string ea;
		decode_RR_table(BITSn(word0,0x0003), rNum);
		assemble_ea_from_MM_table(BITSn(word0,0x000c), rNum, ea);

		std::string SD;
		decode_DDDDD_table(BITSn(word0,0x03e0), SD);
		assemble_arguments_from_W_table(BITSn(word0,0x0400), 'X', SD, ea,
										m_source, m_destination);

		if (SD == "!!") return false;

		m_opcode = "move";
		// NEW // sprintf(opcode_str, "move(c)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 ///////////////////////////////////////
class Movec_2: public Instruction
{
public:
	Movec_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		std::string ea;
		decode_RR_table(BITSn(word0,0x0003), rNum);
		assemble_ea_from_q_table(BITSn(word0,0x0008), rNum, ea);

		std::string SD;
		decode_DDDDD_table(BITSn(word0,0x03e0), SD);
		assemble_arguments_from_W_table(BITSn(word0,0x0400), 'X', SD, ea,
										m_source, m_destination);

		if (SD == "!!") return false;

		m_opcode = "move";
		// NEW // sprintf(opcode_str, "move(c)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 ///////////////////////////////////////
class Movec_3: public Instruction
{
public:
	Movec_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		std::string ea;
		decode_Z_table(BITSn(word0,0x0008), ea);

		std::string SD;
		decode_DDDDD_table(BITSn(word0,0x03e0), SD);
		assemble_arguments_from_W_table(BITSn(word0,0x0400), 'X', SD, ea,
										m_source, m_destination);

		if (SD == "!!") return false;

		m_opcode = "move";
		// NEW // sprintf(opcode_str, "move(c)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 ///////////////////
class Movec_4: public Instruction
{
public:
	Movec_4(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_args = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		char temp[128];
		std::string SD;
		decode_DDDDD_table(BITSn(word0,0x03e0), SD);
		if (SD == "!!") return false;

		std::string ea;
		assemble_ea_from_t_table(BITSn(word0,0x0008), word1, ea);

		// TODO: Figure out what this means, exactly.
		if ((word0 & 0x000c) == 0x000c && (word0 & 0x0400) == 0x0000)
			return false;

		if (BITSn(word0,0x0400))
			sprintf(temp, "%s,%s", ea.c_str(), SD.c_str());
		else
			sprintf(temp, "%s,%s", SD.c_str(), ea.c_str());
		m_args = temp;  // TODO

		m_opcode = "move";
		// NEW // sprintf(opcode_str, "move(c)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_args;	// TODO
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_args;
};

// MOVE(C) : 0010 10dd dddD DDDD : A-144 ///////////////////////////////////////
class Movec_5: public Instruction
{
public:
	Movec_5(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_DDDDD_table(BITSn(word0,0x03e0), m_source);
		decode_DDDDD_table(BITSn(word0,0x001f), m_destination);

		if (m_source == "!!"  || m_destination == "!!") return false;
		if (m_source == "SSH" && m_destination == "SSH") return false;

		m_opcode = "move";
		// NEW // sprintf(opcode_str, "move(c)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE(C) : 0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144 ///////////////////
class Movec_6: public Instruction
{
public:
	Movec_6(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_b = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_b = BITSn(word0,0x00ff);

		std::string SD;
		decode_DDDDD_table(BITSn(word1,0x03e0), SD);
		assemble_reg_from_W_table(BITSn(word1,0x0400), 'X', SD, m_b,
								  m_source, m_destination);

		m_opcode = "move";
		// NEW // m_opcode = "move(c)";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	INT8 m_b;
};

// MOVE(I) : 0010 00DD BBBB BBBB : A-150 ///////////////////////////////////////
class Movei: public Instruction
{
public:
	Movei(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = (INT8)BITSn(word0,0x00ff);

		decode_DD_table(BITSn(word0,0x0300), m_destination);

		m_opcode = "move";
		// NEW // sprintf(opcode_str, "move(i)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		if (m_immediate >= 0) sprintf(temp, "#<+$%x", m_immediate);
		else                  sprintf(temp, "#<-$%x", 1 - m_immediate - 1);
		// NEW // sprintf(temp, "#$%02x,%s", BITSn(word0,0x00ff), D1);

		retString = m_opcode + " " +
					std::string(temp) + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	INT8 m_immediate;
};

// MOVE(M) : 0000 001W RR0M MHHH : A-152 ///////////////////////////////////////
class Movem: public Instruction
{
public:
	Movem(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		decode_RR_table(BITSn(word0,0x00c0), rNum);

		std::string SD;
		std::string ea;
		decode_HHH_table(BITSn(word0,0x0007), SD);
		assemble_ea_from_MM_table(BITSn(word0,0x0018), rNum, ea);
		assemble_arguments_from_W_table(BITSn(word0,0x0100), 'P', SD, ea,
										m_source, m_destination);

		m_opcode = "move";
		// NEW // sprintf(opcode_str, "move(m)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE(M) : 0000 001W RR11 mmRR : A-152 ///////////////////////////////////////
class Movem_2: public Instruction
{
public:
	Movem_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		std::string ea;
		std::string ea2;
		assemble_eas_from_mm_table(BITSn(word0,0x000c), BITSn(word0,0x00c0), BITSn(word0,0x0003), ea, ea2);
		if (BITSn(word0,0x0100))
		{
			m_source = "X:" + ea;
			m_destination = "P:" + ea2;
		}
		else
		{
			m_source = "P:" + ea;
			m_destination = "X:" + ea2;
		}

		m_opcode = "move";
		// NEW // sprintf(opcode_str, "move(m)*");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 ///////////////////
class Movem_3: public Instruction
{
public:
	Movem_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 b;
		b = BITSn(word0,0x00ff);

		std::string SD;
		decode_HHH_table(BITSn(word1,0x0007), SD);
		assemble_reg_from_W_table(BITSn(word1,0x0100), 'P', SD, b,
								  m_source, m_destination);

		m_opcode = "move";
		// NEW // m_opcode = "move(m)";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 2; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE(P) : 0001 100W HH1p pppp : A-156 ///////////////////////////////////////
class Movep: public Instruction
{
public:
	Movep(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		std::string SD;
		decode_HH_table(BITSn(word0,0x00c0), SD);

		std::string fullAddy;
		assemble_address_from_IO_short_address(BITSn(word0,0x001f), fullAddy);
		fullAddy = "<<$" + fullAddy;

		assemble_arguments_from_W_table(BITSn(word0,0x0100), 'X', SD, fullAddy,
										m_source, m_destination);

		m_opcode = "movep";
		// NEW // sprintf(opcode_str, "move(p)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE(P) : 0000 110W RRmp pppp : A-156 ///////////////////////////////////////
class Movep_2: public Instruction
{
public:
	Movep_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		decode_RR_table(BITSn(word0,0x00c0), rNum);

		std::string ea;
		assemble_ea_from_m_table(BITSn(word0,0x0020), rNum, ea);

		std::string fullAddy;	 /* Convert Short Absolute Address to full 16-bit */
		assemble_address_from_IO_short_address(BITSn(word0,0x001f), fullAddy);

		std::string SD;
		SD = "X:<<$" + fullAddy;
		// NEW // sprintf(SD, "X:$%s", fullAddy);

		assemble_arguments_from_W_table(BITSn(word0,0x0100), 'X', SD, ea,
										m_source, m_destination);

		m_opcode = "movep";
		// NEW // sprintf(opcode_str, "move(p)*");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MOVE(S) : 0001 100W HH0a aaaa : A-158 ///////////////////////////////////////
class Moves: public Instruction
{
public:
	Moves(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		std::string SD;
		decode_HH_table(BITSn(word0,0x00c0), SD);

		char temp[32];
		std::string a;
		sprintf(temp, "<$%x", BITSn(word0,0x001f));
		a = temp;

		assemble_arguments_from_W_table(BITSn(word0,0x0100), 'X', SD, a,
										m_source, m_destination);

		m_opcode = "moves";
		// NEW // sprintf(opcode_str, "move(s)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// MPY : .... .... 1k00 FQQQ : A-160 ///////////////////////////////////////////
class Mpy: public Instruction
{
public:
	Mpy(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		/* There are inconsistencies with the S1 & S2 operand ordering in the docs,
           but since it's a multiply it doesn't matter */
		decode_QQQF_table(BITSn(word0,0x07), BITSn(word0,0x08),
						  m_source, m_source2, m_destination);

		std::string sign;
		decode_kSign_table(BITSn(word0,0x40), sign);
		if (sign == "-")
			m_source = sign + m_source; // TODO: Probably silly for Instruction

		m_opcode = "mpy";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MPY : 011m mKKK 1xx0 F0QQ : A-160 ///////////////////////////////////////////
class Mpy_2: public Instruction
{
public:
	Mpy_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQF_table(BITSn(word0,0x03), BITSn(word0,0x08),
						 m_source, m_source2, m_destination);

		m_opcode = "mpy";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MPY : 0001 0110 RRDD FQQQ : A-160 ///////////////////////////////////////////
class Mpy_3: public Instruction
{
public:
	Mpy_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQQF_table(BITSn(word0,0x0007), BITSn(word0,0x0008),
						  m_source, m_source2, m_destination);

		m_opcode = "mpy";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MPYR : .... .... 1k01 FQQQ : A-162 //////////////////////////////////////////
class Mpyr: public Instruction
{
public:
	Mpyr(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		/* There are inconsistencies with the S1 & S2 operand ordering in the docs,
           but since it's a multiply it doesn't matter */
		decode_QQQF_table(BITSn(word0,0x07), BITSn(word0,0x08),
						  m_source, m_source2, m_destination);

		std::string sign;
		decode_kSign_table(BITSn(word0,0x40), sign);
		if (sign == "-")
			m_source = sign + m_source; // TODO: Probably silly for Instruction

		m_opcode = "mpyr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MPYR : 011m mKKK 1--1 F0QQ : A-162 //////////////////////////////////////////
class Mpyr_2: public Instruction
{
public:
	Mpyr_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQF_table(BITSn(word0,0x03), BITSn(word0,0x08),
						 m_source, m_source2, m_destination);

		m_opcode = "mpyr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 ////////////////////////////////////
class Mpysuuu: public Instruction
{
public:
	Mpysuuu(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQF_special_table(BITSn(word0,0x0003), BITSn(word0,0x0008),
								 m_source, m_source2, m_destination);

		std::string a;
		decode_s_table(BITSn(word0,0x0004), a);
		m_opcode = "mpy" + a;
		// NEW // sprintf(opcode_str, "mpy(%s)", A);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// NEG : .... .... 0110 F000 : A-166 ///////////////////////////////////////////
class Neg: public Instruction
{
public:
	Neg(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);

		m_opcode = "neg";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// NEGC : 0001 0101 0110 F000 : A-168 //////////////////////////////////////////
class Negc: public Instruction
{
public:
	Negc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x0008), m_destination);
		m_opcode = "negc";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// NOP : 0000 0000 0000 0000 : A-170 ///////////////////////////////////////////
class Nop: public Instruction
{
public:
	Nop(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "nop";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// NORM : 0001 0101 0010 F0RR : A-172 //////////////////////////////////////////
class Norm: public Instruction
{
public:
	Norm(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x0008), m_destination);

		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x0003), rNum);
		sprintf(temp, "R%d", rNum);
		m_source = temp;

		m_opcode = "norm";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// NOT : .... .... 0110 F001 : A-174 ///////////////////////////////////////////
class Not: public Instruction
{
public:
	Not(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);

		m_opcode = "not";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// OR : .... .... 0010 F1JJ : A-176 ////////////////////////////////////////////
class Or: public Instruction
{
public:
	Or(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_JJF_table(BITSn(word0,0x03),BITSn(word0,0x08),
						 m_source, m_destination);
		m_opcode = "or";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// ORI : 0001 1EE1 iiii iiii : A-178 ///////////////////////////////////////////
class Ori: public Instruction
{
public:
	Ori(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = BITSn(word0,0x00ff);

		decode_EE_table(BITSn(word0,0x0600), m_destination);
		m_opcode = "ori";
		// NEW // sprintf(opcode_str, "or(i)");
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, "#$%x", m_immediate);
		// NEW // sprintf(temp, "#$%02x", BITSn(word0,0x00ff));
		retString = m_opcode + " " + std::string(temp) + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	UINT8 m_immediate;
};

// REP : 0000 0000 111- --RR : A-180 ///////////////////////////////////////////
class Rep: public Instruction
{
public:
	Rep(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x0003), rNum);
		sprintf(temp, "R%d", rNum);
		m_source = temp;

		m_opcode = "rep";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, "X:(%s)", m_source.c_str());
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// REP : 0000 1111 iiii iiii : A-180 ///////////////////////////////////////////
class Rep_2: public Instruction
{
public:
	Rep_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_immediate = 0;
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_immediate = BITSn(word0,0x00ff);
		m_opcode = "rep";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		char temp[32];
		sprintf(temp, "#$%x", m_immediate);
		// NEW // sprintf(temp, "#$%02x (%d)", BITSn(word0,0x00ff), BITSn(word0,0x00ff));
		retString = m_opcode + " " + std::string(temp);
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	UINT8 m_immediate;
};

// REP : 0000 0100 001D DDDD : A-180 ///////////////////////////////////////////
class Rep_3: public Instruction
{
public:
	Rep_3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_DDDDD_table(BITSn(word0,0x001f), m_source);
		if (m_source == "!!") return false;

		m_opcode = "rep";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// REPcc : 0000 0001 0101 cccc : A-184 /////////////////////////////////////////
class Repcc: public Instruction
{
public:
	Repcc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		std::string M;
		decode_cccc_table(BITSn(word0,0x000f), M);
		m_opcode = "rep" + M;
		// NEW // sprintf(opcode_str, "rep.%s", M);
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// RESET : 0000 0000 0000 1000 : A-186 /////////////////////////////////////////
class Reset: public Instruction
{
public:
	Reset(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "reset";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// RND : .... .... 0010 F000 : A-188 ///////////////////////////////////////////
class Rnd: public Instruction
{
public:
	Rnd(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);

		m_opcode = "rnd";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ROL : .... .... 0111 F011 : A-190 ///////////////////////////////////////////
class Rol: public Instruction
{
public:
	Rol(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);

		m_opcode = "rol";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// ROR : .... .... 0111 F010 : A-192 ///////////////////////////////////////////
class Ror: public Instruction
{
public:
	Ror(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);

		m_opcode = "ror";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE; }
};

// RTI : 0000 0000 0000 0111 : A-194 ///////////////////////////////////////////
class Rti: public Instruction
{
public:
	Rti(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "rti";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OUT; }
};

// RTS : 0000 0000 0000 0110 : A-196 ///////////////////////////////////////////
class Rts: public Instruction
{
public:
	Rts(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "rts";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
	size_t flags() { return DASMFLAG_STEP_OUT; }
};

// SBC : .... .... 0101 F01J : A-198 ///////////////////////////////////////////
class Sbc: public Instruction
{
public:
	Sbc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_JF_table(BITSn(word0,0x01), BITSn(word0,0x08),
						m_source, m_destination);

		m_opcode = "sbc";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// STOP : 0000 0000 0000 1010 : A-200 //////////////////////////////////////////
class Stop: public Instruction
{
public:
	Stop(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "stop";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// SUB : .... .... 0100 FJJJ : A-202 ///////////////////////////////////////////
class Sub: public Instruction
{
public:
	Sub(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_JJJF_table(BITSn(word0,0x07), BITSn(word0,0x08),
						  m_source, m_destination);
		m_opcode = "sub";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// SUB : 011m mKKK 0rru Fuuu : A-202 ///////////////////////////////////////////
class Sub_2: public Instruction
{
public:
	Sub_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_uuuuF_table(BITSn(word0,0x17), BITSn(word0,0x08),
						   m_opcode, m_source, m_destination);

		// TODO // m_opcode = "sub";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// SUBL : .... .... 0100 F001 : A-204 //////////////////////////////////////////
class Subl: public Instruction
{
public:
	Subl(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		/* There is only one option for the F table.  This is a very strange opcode. */
		if (!BITSn(word0,0x0008))
		{
			m_source = "B";
			m_destination = "A";
		}
		else
		{
			m_source = "A";
			m_destination = "B";
		}

		m_opcode = "subl";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// SWAP : 0001 0101 0111 F001 : A-206 //////////////////////////////////////////
class Swap: public Instruction
{
public:
	Swap(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x0008), m_destination);

		m_opcode = "swap";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// SWI : 0000 0000 0000 0101 : A-208 ///////////////////////////////////////////
class Swi: public Instruction
{
public:
	Swi(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "swi";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// Tcc : 0001 00cc ccTT Fh0h : A-210 ///////////////////////////////////////////
class Tcc: public Instruction
{
public:
	Tcc(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_destination2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_h0hF_table(BITSn(word0,0x0007),BITSn(word0,0x0008),
						  m_source, m_destination);

		INT8 rNum;
		char temp[32];
		decode_RR_table(BITSn(word0,0x0030), rNum);
		sprintf(temp, "R%d", rNum);
		m_destination2 = temp;

		std::string M;
		decode_cccc_table(BITSn(word0,0x03c0), M);
		m_opcode = "t" + M;
		// NEW // sprintf(opcode_str, "t.%s", M);
		if (m_source != m_destination)
			return true;
		if (m_destination2 != "R0")
			return true;

		return false;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
		if (m_source != m_destination)
			retString += std::string(" ") + m_source + "," + m_destination;

		if (m_destination2 != "R0")
			retString += std::string(" R0,") + m_destination2;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_destination2;
};

// TFR : .... .... 0001 FJJJ : A-212 ///////////////////////////////////////////
class Tfr: public Instruction
{
public:
	Tfr(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_JJJF_table(BITSn(word0,0x07), BITSn(word0,0x08),
						  m_source, m_destination);

		m_opcode = "tfr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// TFR : 011m mKKK 0rr1 F0DD : A-212 ///////////////////////////////////////////
class Tfr_2: public Instruction
{
public:
	Tfr_2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_DDF_table(BITSn(word0,0x03), BITSn(word0,0x08),
						 m_source, m_destination);

		m_opcode = "tfr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// TFR(2) : 0001 0101 0000 F00J : A-214 ////////////////////////////////////////
class Tfr2: public Instruction
{
public:
	Tfr2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_JF_table(BITSn(word0,0x0001),BITSn(word0,0x0008),
						m_destination, m_source);

		m_opcode = "tfr2";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// TFR(3) : 0010 01mW RRDD FHHH : A-216 ////////////////////////////////////////
class Tfr3: public Instruction
{
public:
	Tfr3(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_destination2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_DDF_table(BITSn(word0,0x0030), BITSn(word0,0x0008),
						 m_destination, m_source);

		std::string SD;
		decode_HHH_table(BITSn(word0,0x0007), SD);
		// If the destination of the second move is the same as the first, you're invalid
		if (SD == m_destination && BITSn(word0,0x100)) return false;

		INT8 rNum;
		decode_RR_table(BITSn(word0,0x00c0), rNum);

		std::string ea;
		assemble_ea_from_m_table(BITSn(word0,0x0200), rNum, ea);

		assemble_arguments_from_W_table(BITSn(word0,0x0100), 'X', SD, ea,
										m_source2, m_destination2);

		m_opcode = "tfr3";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_destination + " " +
					m_source2 + "," + m_destination2;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
	std::string m_destination2;
};

// TST : .... .... 0010 F001 : A-218 ///////////////////////////////////////////
class Tst: public Instruction
{
public:
	Tst(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x08), m_destination);

		m_opcode = "tst";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_NONE; }
};

// TST(2) : 0001 0101 0001 -1DD : A-220 ////////////////////////////////////////
class Tst2: public Instruction
{
public:
	Tst2(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_DD_table(BITSn(word0,0x0003), m_source);

		m_opcode = "tst2";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_source;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// WAIT : 0000 0000 0000 1011 : A-222 //////////////////////////////////////////
class Wait: public Instruction
{
public:
	Wait(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		m_opcode = "wait";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// ZERO : 0001 0101 0101 F000 : A-224 //////////////////////////////////////////
class Zero: public Instruction
{
public:
	Zero(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_F_table(BITSn(word0,0x0008), m_destination);

		m_opcode = "zero";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }
};

// SHFL : 0001 0101 1101 FQQQ : !!UNDOCUMENTED!! ///////////////////////////////
class Shfl: public Instruction
{
public:
	Shfl(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQQF_table(BITSn(word0,0x0007), BITSn(word0,0x0008),
						  m_source, m_source2, m_destination);

		// This hackery amounts to a very strange QQQF table...
		if (m_source == "X0" && m_source2 == "X0") return false;
		if (m_source == "X1" && m_source2 == "X0") return false;

		if (m_source == "Y0" && m_source2 == "X1")
		{
			m_source  = "X1";
			m_source2 = "Y0";
		}
		if (m_source == "Y1" && m_source2 == "X1")
		{
			m_source  = "X1";
			m_source2 = "Y1";
		}

		m_opcode = "shfl";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

// SHFR : 0001 0101 1111 FQQQ : !!UNDOCUMENTED!! ///////////////////////////////
class Shfr: public Instruction
{
public:
	Shfr(const Opcode* oco, const UINT16 word0, const UINT16 word1) : Instruction(oco)
	{
		m_source2 = "";
		m_valid = decode(word0, word1);
	}
	bool decode(const UINT16 word0, const UINT16 word1)
	{
		decode_QQQF_table(BITSn(word0,0x0007), BITSn(word0,0x0008),
						  m_source, m_source2, m_destination);

		// This hackery amounts to a very strange QQQF table...
		if (m_source == "X0" && m_source2 == "X0") return false;
		if (m_source == "X1" && m_source2 == "X0") return false;

		if (m_source == "Y0" && m_source2 == "X1")
		{
			m_source  = "X1";
			m_source2 = "Y0";
		}
		if (m_source == "Y1" && m_source2 == "X1")
		{
			m_source  = "X1";
			m_source2 = "Y1";
		}

		m_opcode = "shfr";
		return true;
	}
	void disassemble(std::string& retString) const
	{
		retString = m_opcode + " " +
					m_source + "," +
					m_source2 + "," + m_destination;
	}
	void evaluate(dsp56k_core* cpustate) {}
	size_t size() const { return 1; }
	size_t accumulatorBitsModified() const { return BM_HIGH | BM_MIDDLE | BM_LOW; }

private:
	std::string m_source2;
};

}
#endif
