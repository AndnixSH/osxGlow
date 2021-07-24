//osxGlow
//simple glow demo :)
//yeah it's a little bit paste-y, and the code could be cleaner, but whatever



//includes

#include <thread>

#include <mach-o/dyld.h>
#include <sys/stat.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/mach_init.h>



//"sdk"

//https://github.com/perilouswithadollarsign/cstrike15_src/blob/29e4c1fda9698d5cebcdaf1a0de4b829fa149bf8/game/client/glow_outline_effect.h#L23
enum GlowRenderStyle_t
{
    GLOWRENDERSTYLE_DEFAULT = 0,
    GLOWRENDERSTYLE_RIMGLOW3D,
    GLOWRENDERSTYLE_EDGE_HIGHLIGHT,
    GLOWRENDERSTYLE_EDGE_HIGHLIGHT_PULSE,
    GLOWRENDERSTYLE_COUNT,
};

//lol
class Vector
{
public:
    float x, y, z;
    Vector(float X, float Y, float Z) {
        x = X; y = Y; z = Z;
    };
};



//detour hook
//very basic, no trampoline neeeded

#define size_of_jump (mach_msg_type_number_t)(sizeof(uintptr_t) * 2)
class detour {
private:
    void* address;
    uint8_t opcodes_copy[size_of_jump];
    kern_return_t get_memory_protection(void* address, vm_prot_t &prot) {
        mach_vm_address_t addr = (mach_vm_address_t)address;
        mach_vm_size_t vmsize;
        vm_region_flavor_t flavor = VM_REGION_BASIC_INFO_64;
        vm_region_basic_info_data_64_t info;
        mach_msg_type_number_t infoCount = VM_REGION_BASIC_INFO_COUNT_64;
        mach_port_t object;
        kern_return_t err = mach_vm_region(mach_task_self(), &addr, &vmsize, flavor, (vm_region_info_64_t)&info, &infoCount, &object);
        
        if (err == KERN_SUCCESS)
            prot = info.protection;
        return err;
    }
public:
    kern_return_t hook(void* func_addr, void* func) {
        address = func_addr;
        
        //thanks https://github.com/rodionovd/rd_route
        uint8_t opcodes[size_of_jump];
        
        opcodes[0] = 0xFF;
        opcodes[1] = 0x25;
        *((int*)&opcodes[2]) = 0;
        *((uintptr_t*)&opcodes[6]) = (uintptr_t)func;
        
        kern_return_t err = KERN_SUCCESS;
        
        vm_prot_t prot;
        err = get_memory_protection(address, prot);
        if (err != KERN_SUCCESS)
            return err;
        
        err = mach_vm_protect(mach_task_self(), (mach_vm_address_t)address, (mach_vm_size_t)size_of_jump, FALSE, VM_PROT_ALL);
        if (err != KERN_SUCCESS)
            return err;
        
        memcpy(&opcodes_copy, address, size_of_jump);
        
        memcpy(address, &opcodes, size_of_jump);
        
        err = mach_vm_protect(mach_task_self(), (mach_vm_address_t)address, (mach_vm_size_t)size_of_jump, FALSE, prot);
        
        return err;
    }
    kern_return_t unhook() {
        kern_return_t err = KERN_SUCCESS;
        
        vm_prot_t prot;
        err = get_memory_protection(address, prot);
        if (err != KERN_SUCCESS)
            return err;
        
        err = mach_vm_protect(mach_task_self(), (mach_vm_address_t)address, (mach_vm_size_t)size_of_jump, FALSE, VM_PROT_ALL);
        if (err != KERN_SUCCESS)
            return err;
        
        memcpy(address, &opcodes_copy, size_of_jump);
        
        err = mach_vm_protect(mach_task_self(), (mach_vm_address_t)address, (mach_vm_size_t)size_of_jump, FALSE, prot);
        
        return err;
    }
};
#undef size_of_jump



//pattern scanner

uintptr_t scan(const char* module, unsigned char* sig, const char* mask) {
    //thanks https://github.com/scen/libembryo
    unsigned int count = _dyld_image_count();
    for (int i = 0; i < count; i++) {
        const char* image = _dyld_get_image_name(i);
        if (!strstr(image, module))
            continue;
            
        struct stat sb;
        if (stat(image, &sb))
            continue;
            
        mach_header_64* header = (mach_header_64*)_dyld_get_image_header(i);
        uintptr_t address = (uintptr_t)header;
        std::size_t size = sb.st_size;
        
        //thanks https://github.com/AimTuxOfficial/AimTux/
        auto compare = [](const unsigned char* data, const unsigned char* sig, const char* mask) {
            for (; *mask; ++mask, ++data, ++sig) {
                if (*mask == 'x' && *data != *sig)
                    return false;
            }
            
            return (*mask) == 0;
        };
        
        for (uintptr_t i = 0; i < size; i++) {
            if (compare((unsigned char*)(address + i), sig, mask))
                return (uintptr_t)(address + i);
        }
    }
    
    return 0;
}



//hook

//https://www.unknowncheats.me/forum/counterstrike-global-offensive/427787-easy-alternative-internal-glow-basically-3-lines.html
bool hooked_GlowEffectSpectator(void* thisPlayer, void* pLocalPlayer, GlowRenderStyle_t& glowStyle, Vector& glowColor, float& alphaStart, float& alpha, float& timeStart, float& timeTarget, bool& animate) {
    
    if (!pLocalPlayer)
        return false;
    
    glowStyle = GLOWRENDERSTYLE_DEFAULT;
    
    glowColor = {
        242.0f / 255.0f,
        117.0f / 255.0f,
        117.0f / 255.0f
    };
    
    alpha = 1.f;
    
    return true;
}



//main

detour hooker;

int __attribute__((constructor)) start() {
    std::thread mainThread([](){
        uintptr_t func_addr = scan("osx64/client.dylib", (unsigned char*)"\x55\x48\x89\xE5\x41\x57\x41\x56\x41\x55\x41\x54\x53\x48\x83\xEC\x38\x48\x89\xCB\x49\x89\xF4", "xxxxxxxxxxxxxxxxxxxxxxx");
        
        if (func_addr)
            hooker.hook((void*)func_addr, (void*)hooked_GlowEffectSpectator);
    });
    mainThread.detach();
    
    return 0;
}

void __attribute__((destructor)) shutdown() {
    hooker.unhook();
}
