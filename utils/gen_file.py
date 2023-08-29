import sys
section_size = 512

def gen_file(files, target, header):
  tgt = open(target, "ab+")
  hdr = open(header, "w")
  sec_start=201
  for f in files:
    with open(f, "rb") as e:
      elf = e.read()
      elf += b'\0' * (section_size - (len(elf) % section_size))
      # print(f"size of {e.name} is {len(elf)}")
      tgt.write(elf)
      hdr.write('{"%s", %d, %d},\n' % (f.split('/')[-1], sec_start, len(elf) // section_size))
      sec_start += len(elf) // section_size
  tgt.close()
  hdr.close()

if __name__ == '__main__':
    gen_file(sys.argv[3:], sys.argv[1], sys.argv[2])