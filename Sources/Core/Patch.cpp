/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "CPU.h"
#include "Internals.h"
#include "Local.hpp"
#include "Patch.h"

using namespace std;

namespace {

typedef struct _PatchWrapper {
   uint32 signature;
   bool active;

   MEMORY_PATCH patch;

} PatchWrapper;

typedef vector<PatchWrapper> PatchTableType;
PatchTableType patchTable;

force_inline uint32 GetSignature(const MEMORY_PATCH& patch)
{
   return (patch.address << 16) | (patch.value << 8) | patch.match_value;
}

/* We sort the patch table by enabled status, so that only patches that
   are actually used will degrade performance. */
constant_function bool PatchTableSorter(const PatchWrapper first, const PatchWrapper second) {
	return first.patch.enabled;
}

} // namespace anonymous

BOOL add_patch(const MEMORY_PATCH* patch)
{
   Safeguard(patch);

   if(patchTable.size() >= MAX_PATCHES)
      return FALSE;

   PatchWrapper wrapper;
   wrapper.signature = GetSignature(*patch);
   wrapper.active = false;

   memcpy(&wrapper.patch, patch, sizeof(MEMORY_PATCH));

   patchTable.push_back(wrapper);
   sort(patchTable.begin(), patchTable.end(), PatchTableSorter);

   return TRUE;
}

void remove_patch(const MEMORY_PATCH* patch)
{
   Safeguard(patch);

   const uint32 signature = GetSignature(*patch);

   for(PatchTableType::iterator i = patchTable.begin(); i != patchTable.end(); ) {
      const PatchWrapper& wrapper = *i;

      if(wrapper.signature == signature) {
         i = patchTable.erase(i);
         continue;
      }

      i++;
   }
}

MEMORY_PATCH* get_patch(const int index)
{
   Safeguard(index >= 0);

   if((size_type)index >= patchTable.size()) {
      // We've reached the end of the patch list, so let the caller know.
      return NULL;
   }

   return &patchTable[index].patch;
}

void map_patches(const UINT16 start_address, const UINT16 end_address)
{
   const size_type count = patchTable.size();
   if(count == 0) {
      // No patches are currently available.
      return;
   }

   for(size_type i = 0; i < count; i++) {
      PatchWrapper& wrapper = patchTable[i];

      // Get a direct reference to the patch to keep this clean.
      const MEMORY_PATCH& patch = wrapper.patch;

      // Check to make sure the patch is enabled.
      if(!patch.enabled) {
         /* Since the list is already sorted by enabled status, we bail
            out on the first disabled patch. */
         return;
      }

      // Check to make sure the address is in range.
      if((patch.address < start_address) ||
         (patch.address > end_address))
         continue;

      /* Due to how the NES Game Genie works, we have to deactivate the
         patch if it was already active. This is because the address
         alone is not enough to apply a patch; due to the use of mappers
         a test against a "match value" is also required. */
      if(wrapper.active) {
         wrapper.active = false;
         cpu__read_patch[patch.address] = 0;
      }

      // Fetch the current byte from memory.
      const uint8 value = cpu_read(patch.address);

      /* Compare it against the match value. If it matches, store the
         difference between them in the CPU I/O table. */
      if(patch.match_value == value) {
         wrapper.active = true;
         cpu__read_patch[patch.address] = patch.value - value;
      }
   }
}
