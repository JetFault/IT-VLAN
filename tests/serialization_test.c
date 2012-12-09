#include "../vlanpacket.h"
#include <stdlib.h>
#include <string.h>

#include <assert.h>

int main() {
  struct leave_packet pack = { .packet_type = PACKET_TYPE_LEAVE, .packet_length = 20, 
    .local = { .ip = 123456, .port = 1991, .mac_addr = "macadd"}, .ID=1337133713371337};

  size_t header_size = sizeof(uint16_t) * 2;
  char* buffer;
  serialize(pack.packet_type, &pack, &buffer);

  struct leave_packet * deser_pack;

  deserialize(buffer, (void **)&deser_pack);

  assert(pack.packet_type == deser_pack->packet_type);
  assert(pack.packet_length == deser_pack->packet_length);
  assert(pack.local.ip == deser_pack->local.ip);
  assert(pack.local.ip == deser_pack->local.ip);
  assert(strncmp(pack.local.mac_addr, deser_pack->local.mac_addr, 6) == 0);
  assert(pack.ID == deser_pack->ID);
}
