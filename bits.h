#ifndef __BITS_H__
#define __BITS_H__

/* define for all the bits in a command */

/* 0-1 */
#define TYPE_SHIFT (0)
#define TYPE_MASK (0x0003 << TYPE_SHIFT)
/* 2-3 */
#define DESTINATION_ADDRESSING_METHOD_SHIFT (2)
#define DESTINATION_ADDRESSING_METHOD_MASK (0x0003 << DESTINATION_ADDRESSING_METHOD_SHIFT)

/* 4-5 */
#define SOURCE_ADDRESSING_METHOD_SHIFT (4)
#define SOURCE_ADDRESSING_METHOD_MASK (0x0003 << SOURCE_ADDRESSING_METHOD_SHIFT)
/* 6-9 */
#define OPCODE_SHIFT (6)
#define OPCODE_MASK (0x000F << OPCODE_SHIFT)

/* define for all the bits in label address */

/* 2-7 address*/
#define ADDRESS_SHIFT (2)
#define ADDRESS_MASK (0x00FF << ADDRESS_SHIFT)

/* define for all the bits in register address */

/* 2-5 */
#define DESTINATION_REGISTER_SHIFT (2)
#define DESTINATION_REGISTER_MASK (0x0007 << DESTINATION_REGISTER_SHIFT)
/* 6-9 */
#define SOURCE_REGISTER_SHIFT (6)
#define SOURCE_REGISTER_MASK (0x0007 << SOURCE_REGISTER_SHIFT)


#endif /* __BITS__ */
