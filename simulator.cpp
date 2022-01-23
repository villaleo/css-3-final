/**
 * @file simulator.cpp
 * @brief Implementation for the simulator
 * @author Leonardo Villalobos
 * @date 12/07/2021
 *
 */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <cstdlib>
#include <cmath>
#include <map>
#include <algorithm>

/** The global namespace for the project */
namespace global
{
    /*
     * Stores the instruction string.
     *                        ________________
     * Stores in the form 0b |00000|000000|00| (13 bits)
     *                       -----------------
     *                        ^      ^     ^
     *                     opcode  addr.  reg.
     *
     * 5 bits are used for opcode -> 0b00000
     * 6 bits are used for address -> 0b000000
     * 2 bits are used for register -> 0b000
     */
    std::string instruction;

    std::string opcode;                                      // Stores the opcode of the instruction
    std::map<std::string, unsigned int> registers;           // A map from strings to unsigned ints for registers
    std::map<std::string, std::vector<unsigned int>> arrays; // A map from strings to vectors of unsigned ints for arrays
    std::list<std::string> memory;                           // A list of strings used for memory

    enum Opcode : unsigned int
    {
        Stop = 0b00000, // Stop -- Terminate the program
        In,             // In <dest.> -- Input value from the keyboard
        Out,            // Out <src.> -- Output value to the screen
        Incr,           // Incr <src.> <amt.> -- Increment the value at the address by an amount
        Add,            // Add <lhs> <rhs> <dest.> -- Add the value at two address and store result at last address
        Sub,            // Sub <lhs> <rhs> <dest.> -- Subtract the value at two address and store result at last address
        Mul,            // Mul <lhs> <rhs> <dest.> -- Multiply the value at two address and store result at last address
        List,           // List <amt.> <src.> -- Create an array and store at an address
        ListInit,       // ListInit <src.> -- Initialize an array; reads the values from the keyboard
        ListSum,        // ListSum <src.> <dest.> -- Sum the values in an array
        TidyUp,         // Clear all regsiters
    };

    /**
     * A function used to check if a string is a valid register,
     * that is either __REG_1 ("00"), __REG_2 ("01"), __REG_3 ("10"), or __REG_4 ("11").
     * For example, "00" is a valid register.
     * So validRegisters("00") returns true.
     *
     * @brief Checks if a string is a valid register.
     *
     * @param reg The string to check (the register)
     * @return true if the string is a valid register, false otherwise
     */
    bool validRegister(const std::string &reg)
    {
        auto validSize = reg.size() == 2;                                                       // Register size must be 2
        auto validChars = (reg[0] == '0' || reg[0] == '1') && (reg[1] == '0' || reg[1] == '1'); // Must be 0 or 1

        if (!(validSize && validChars))
        {
            std::cerr << "Error: Invalid destination register \'" << reg << "\'.\n";
            return false;
        }
        return true;
    }
}

/**
 * Given a binary number as a string, returns the corresponding integer.
 * For example, given "0b00001", returns 1.
 *
 * @brief Converts a binary number to an integer.
 *
 * @param binary The binary number as a string
 * @return The integer corresponding to the binary number
 */
unsigned int binaryToDecimal(const std::string &binary);

int main()
{
    using namespace global;

    std::ifstream inputFile("benchmarkBinary.txt");
    if (inputFile.fail())
    {
        std::cerr << "Error: Could not open file." << std::endl;
        return EXIT_FAILURE;
    }

    // Initialize registers
    registers["00"] = 0; // __REG_0
    registers["01"] = 0; // __REG_1
    registers["10"] = 0; // __REG_2
    registers["11"] = 0; // __REG_3

    // Read the file
    while (inputFile >> instruction)
        memory.push_back(instruction);

    inputFile.close(); // Close the file

    // Preprocess the memory
    // Check if Stop instruction is present
    if (std::find(memory.begin(), memory.end(), "0000000000000") == memory.end())
    {
        std::cerr << "Error: Could not find the \'Stop\' instruction." << std::endl;
        return EXIT_FAILURE;
    }

    // Iterate through the memory
    for (std::string &ins : memory)
    {
        // Check if opcodes are valid
        if (binaryToDecimal(ins.substr(0, 5)) > Opcode::TidyUp)
        {
            std::cerr << "Error: Invalid opcode \'" << ins.substr(0, 5) << "\'\n";
            return EXIT_FAILURE;
        }
    }

    // Begin execution
    for (auto it = memory.begin(); it != memory.end(); ++it)
    {
        instruction = *it;
        opcode = instruction.substr(0, 5);

        if (binaryToDecimal(opcode) == Opcode::Stop)
        {
            std::cout << "Program ended successfully.\n";
        }
        else if (binaryToDecimal(opcode) == 1)
        {
            std::cout << "Enter a value: ";
            unsigned int value{0};
            std::cin >> value;

            std::string destination = instruction.substr(5, 2);

            if (!validRegister(destination))
                return EXIT_FAILURE;

            registers[destination] = value;
        }
        else if (binaryToDecimal(opcode) == Opcode::Out)
        {
            auto source = instruction.substr(5, 2);

            if (!validRegister(source))
                return EXIT_FAILURE;

            std::cout << registers[source] << std::endl;
        }
        else if (binaryToDecimal(opcode) == Opcode::Incr)
        {
            auto amount = binaryToDecimal(instruction.substr(5, 6));
            auto source = instruction.substr(11);

            if (!validRegister(source))
                return EXIT_FAILURE;

            registers[source] += amount;
        }
        else if (binaryToDecimal(opcode) == Opcode::Add)
        {
            std::string lhs, rhs, destination;
            lhs = instruction.substr(5, 2);
            rhs = instruction.substr(7, 2);
            destination = instruction.substr(9, 2);

            if (!validRegister(destination))
                return EXIT_FAILURE;

            registers[destination] = registers[lhs] + registers[rhs];
        }
        else if (binaryToDecimal(opcode) == Opcode::Sub)
        {
            std::string lhs, rhs, destination;
            lhs = instruction.substr(5, 2);
            rhs = instruction.substr(7, 2);
            destination = instruction.substr(9, 2);

            if (!validRegister(destination))
                return EXIT_FAILURE;

            registers[destination] = registers[lhs] - registers[rhs];
        }
        else if (binaryToDecimal(opcode) == Opcode::Mul)
        {
            std::string lhs, rhs, destination;
            lhs = instruction.substr(5, 2);
            rhs = instruction.substr(7, 2);
            destination = instruction.substr(9, 2);

            if (!validRegister(destination))
                return EXIT_FAILURE;

            registers[destination] = registers[lhs] * registers[rhs];
        }
        else if (binaryToDecimal(opcode) == Opcode::List)
        {
            std::size_t amount;
            if (instruction.substr(7, 4) == "0000") // Size is in a register
            {
                auto registerAddress = instruction.substr(5, 2);

                if (!validRegister(registerAddress))
                    return EXIT_FAILURE;

                amount = registers[registerAddress];
            }
            else // Size is a literal
                amount = static_cast<std::size_t>(binaryToDecimal(instruction.substr(5, 6)));
            auto source = instruction.substr(11);

            if (!validRegister(source))
                return EXIT_FAILURE;

            arrays[source] = std::vector<unsigned int>(amount);
        }
        else if (binaryToDecimal(opcode) == Opcode::ListInit)
        {
            auto source = instruction.substr(5, 2);

            if (!validRegister(source))
                return EXIT_FAILURE;

            for (std::size_t i = 0; i < arrays[source].size(); ++i)
            {
                std::cout << "Enter value for index " << i << ": ";
                std::cin >> arrays[source][i];
            }
        }
        else if (binaryToDecimal(opcode) == Opcode::ListSum)
        {
            auto source = instruction.substr(5, 2);
            auto destination = instruction.substr(7, 2);
            unsigned int sum{0};

            if (!(validRegister(source) && validRegister(destination)))
                return EXIT_FAILURE;

            for (unsigned int value : arrays[source])
                sum += value;
            registers[destination] = sum;
        }
        else if (binaryToDecimal(opcode) == Opcode::TidyUp)
        {
            for (auto &reg : registers)
                reg.second = 0;
        }
    }
    return EXIT_SUCCESS;
}

unsigned int binaryToDecimal(const std::string &binary)
{
    unsigned int result{0};
    for (std::size_t i = 0; i < binary.length(); ++i)
        if (binary[i] == '1')
            result += pow(2, binary.length() - 1 - i);
    return result;
}