/*********************IMPORTANT DRAKVUF LICENSE TERMS***********************
 *                                                                         *
 * DRAKVUF (C) 2014-2017 Tamas K Lengyel.                                  *
 * Tamas K Lengyel is hereinafter referred to as the author.               *
 * This program is free software; you may redistribute and/or modify it    *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; Version 2 ("GPL"), BUT ONLY WITH ALL OF THE   *
 * CLARIFICATIONS AND EXCEPTIONS DESCRIBED HEREIN.  This guarantees your   *
 * right to use, modify, and redistribute this software under certain      *
 * conditions.  If you wish to embed DRAKVUF technology into proprietary   *
 * software, alternative licenses can be aquired from the author.          *
 *                                                                         *
 * Note that the GPL places important restrictions on "derivative works",  *
 * yet it does not provide a detailed definition of that term.  To avoid   *
 * misunderstandings, we interpret that term as broadly as copyright law   *
 * allows.  For example, we consider an application to constitute a        *
 * derivative work for the purpose of this license if it does any of the   *
 * following with any software or content covered by this license          *
 * ("Covered Software"):                                                   *
 *                                                                         *
 * o Integrates source code from Covered Software.                         *
 *                                                                         *
 * o Reads or includes copyrighted data files.                             *
 *                                                                         *
 * o Is designed specifically to execute Covered Software and parse the    *
 * results (as opposed to typical shell or execution-menu apps, which will *
 * execute anything you tell them to).                                     *
 *                                                                         *
 * o Includes Covered Software in a proprietary executable installer.  The *
 * installers produced by InstallShield are an example of this.  Including *
 * DRAKVUF with other software in compressed or archival form does not     *
 * trigger this provision, provided appropriate open source decompression  *
 * or de-archiving software is widely available for no charge.  For the    *
 * purposes of this license, an installer is considered to include Covered *
 * Software even if it actually retrieves a copy of Covered Software from  *
 * another source during runtime (such as by downloading it from the       *
 * Internet).                                                              *
 *                                                                         *
 * o Links (statically or dynamically) to a library which does any of the  *
 * above.                                                                  *
 *                                                                         *
 * o Executes a helper program, module, or script to do any of the above.  *
 *                                                                         *
 * This list is not exclusive, but is meant to clarify our interpretation  *
 * of derived works with some common examples.  Other people may interpret *
 * the plain GPL differently, so we consider this a special exception to   *
 * the GPL that we apply to Covered Software.  Works which meet any of     *
 * these conditions must conform to all of the terms of this license,      *
 * particularly including the GPL Section 3 requirements of providing      *
 * source code and allowing free redistribution of the work as a whole.    *
 *                                                                         *
 * Any redistribution of Covered Software, including any derived works,    *
 * must obey and carry forward all of the terms of this license, including *
 * obeying all GPL rules and restrictions.  For example, source code of    *
 * the whole work must be provided and free redistribution must be         *
 * allowed.  All GPL references to "this License", are to be treated as    *
 * including the terms and conditions of this license text as well.        *
 *                                                                         *
 * Because this license imposes special exceptions to the GPL, Covered     *
 * Work may not be combined (even as part of a larger work) with plain GPL *
 * software.  The terms, conditions, and exceptions of this license must   *
 * be included as well.  This license is incompatible with some other open *
 * source licenses as well.  In some cases we can relicense portions of    *
 * DRAKVUF or grant special permissions to use it in other open source     *
 * software.  Please contact tamas.k.lengyel@gmail.com with any such       *
 * requests.  Similarly, we don't incorporate incompatible open source     *
 * software into Covered Software without special permission from the      *
 * copyright holders.                                                      *
 *                                                                         *
 * If you have any questions about the licensing restrictions on using     *
 * DRAKVUF in other works, are happy to help.  As mentioned above,         *
 * alternative license can be requested from the author to integrate       *
 * DRAKVUF into proprietary applications and appliances.  Please email     *
 * tamas.k.lengyel@gmail.com for further information.                      *
 *                                                                         *
 * If you have received a written license agreement or contract for        *
 * Covered Software stating terms other than these, you may choose to use  *
 * and redistribute Covered Software under those terms instead of these.   *
 *                                                                         *
 * Source is provided to this software because we believe users have a     *
 * right to know exactly what a program is going to do before they run it. *
 * This also allows you to audit the software for security holes.          *
 *                                                                         *
 * Source code also allows you to port DRAKVUF to new platforms, fix bugs, *
 * and add new features.  You are highly encouraged to submit your changes *
 * on https://github.com/tklengyel/drakvuf, or by other methods.           *
 * By sending these changes, it is understood (unless you specify          *
 * otherwise) that you are offering unlimited, non-exclusive right to      *
 * reuse, modify, and relicense the code.  DRAKVUF will always be          *
 * available Open Source, but this is important because the inability to   *
 * relicense code has caused devastating problems for other Free Software  *
 * projects (such as KDE and NASM).                                        *
 * To specify special license conditions of your contributions, just say   *
 * so when you send them.                                                  *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the DRAKVUF   *
 * license file for more details (it's in a COPYING file included with     *
 * DRAKVUF, and also available from                                        *
 * https://github.com/tklengyel/drakvuf/COPYING)                           *
 *                                                                         *
 ***************************************************************************/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <dirent.h>
#include <glib.h>
#include <err.h>
#include <assert.h>

#include <libvmi/libvmi.h>
#include "plugins/plugins.h"
#include "private.h"
#include "filetracer.h"

static event_response_t objattr_read(drakvuf_t drakvuf, drakvuf_trap_info_t* info, addr_t attr)
{
    if ( !attr )
        return 0;

    const char* syscall_name = info->trap->name;
    filetracer* f = (filetracer*)info->trap->data;
    vmi_instance_t vmi = drakvuf_lock_and_get_vmi(drakvuf);

    access_context_t ctx;
    ctx.translate_mechanism = VMI_TM_PROCESS_DTB;
    ctx.dtb = info->regs->cr3;

    addr_t file_root_handle = 0;
    ctx.addr = attr + f->objattr_root;
    if ( VMI_FAILURE == vmi_read_addr(vmi, &ctx, &file_root_handle) )
    {
        drakvuf_release_vmi(drakvuf);
        return 0;
    }

    char* file_root = drakvuf_get_filename_from_handle(drakvuf, info, file_root_handle);

    ctx.addr = attr + f->objattr_name;
    if ( VMI_FAILURE == vmi_read_addr(vmi, &ctx, &ctx.addr) )
    {
        g_free(file_root);
        drakvuf_release_vmi(drakvuf);
        return 0;
    }

    unicode_string_t* file_name_us = drakvuf_read_unicode(drakvuf, info, ctx.addr);
    drakvuf_release_vmi(drakvuf);

    if ( !file_name_us )
    {
        g_free(file_root);
        return 0;
    }

    uint32_t file_attr = 0;
    ctx.addr = attr + f->objattr_attr;
    if ( VMI_FAILURE == vmi_read_32(vmi, &ctx, &file_attr) )
    {
        g_free(file_root);
        drakvuf_release_vmi(drakvuf);
        return 0;
    }

    char* file_path = g_strdup_printf("%s%s%s",
                                      file_root ?: "",
                                      file_root ? "\\" : "",
                                      file_name_us->contents);

    vmi_free_unicode_str(file_name_us);
    g_free(file_root);

    switch (f->format)
    {
        case OUTPUT_CSV:
            printf("filetracer," FORMAT_TIMEVAL ",%" PRIu32 ",0x%" PRIx64 ",\"%s\",%" PRIi64",%s,%s,0x%" PRIu32 "\n",
                   UNPACK_TIMEVAL(info->timestamp), info->vcpu, info->regs->cr3, info->proc_data.name, info->proc_data.userid, syscall_name, file_path, file_attr);
            break;

        case OUTPUT_KV:
            printf("filetracer Time=" FORMAT_TIMEVAL ",PID=%d,PPID=%d,ProcessName=\"%s\",Method=%s,File=\"%s\",Attributes=0x%x\n",
                   UNPACK_TIMEVAL(info->timestamp), info->proc_data.pid, info->proc_data.ppid, info->proc_data.name,
                   syscall_name, file_path, file_attr);
            break;

        default:
        case OUTPUT_DEFAULT:
            printf("[FILETRACER] TIME:" FORMAT_TIMEVAL " VCPU:%" PRIu32 " CR3:0x%" PRIx64 ",\"%s\" %s:%" PRIi64 " %s,%s,0x%" PRIu32 "\n",
                   UNPACK_TIMEVAL(info->timestamp), info->vcpu, info->regs->cr3, info->proc_data.name,
                   USERIDSTR(drakvuf), info->proc_data.userid, syscall_name, file_path, file_attr);
            break;
    }

    g_free(file_path);

    return 0;
}

static bool is_absolute_path(char const* file_name)
{
    // TODO: need more strong way determing that the @file_name is absolute path.
    return file_name && file_name[0] == '\\';
}

static char* get_parent_folder(char const* file_name)
{
    // TODO: need more strong way getting parent folder for @file_name.
    if (file_name)
    {
        char* end = g_strrstr(file_name, "\\");
        if (end && end != file_name)
            return g_strndup(file_name, end - file_name);
    }
    return g_strdup("\\");
}


static void print_rename_file_info(vmi_instance_t vmi, drakvuf_t drakvuf, drakvuf_trap_info_t* info, addr_t src_file_handle, addr_t fileinfo)
{
    filetracer* f = (filetracer*)info->trap->data;
    const char* syscall_name = info->trap->name;
    const char* operation_name = "FileRenameInformation";

    access_context_t ctx;
    ctx.translate_mechanism = VMI_TM_PROCESS_DTB;
    ctx.dtb = info->regs->cr3;

    addr_t dst_file_root_handle = 0;
    ctx.addr = fileinfo + f->newfile_root_offset;
    if ( VMI_FAILURE == vmi_read_addr(vmi, &ctx, &dst_file_root_handle) )
        return;

    uint32_t dst_file_name_length = 0;
    ctx.addr = fileinfo + f->newfile_name_length_offset;
    if ( VMI_FAILURE == vmi_read_32(vmi, &ctx, &dst_file_name_length) )
        return;

    // convert length in bytes to length in wchar symbols
    dst_file_name_length /= 2;

    ctx.addr = fileinfo + f->newfile_name_offset;
    unicode_string_t* dst_file_name_us = drakvuf_read_wchar_array(vmi, &ctx, dst_file_name_length);
    if ( !dst_file_name_us )
        return;

    char* src_file = drakvuf_get_filename_from_handle(drakvuf, info, src_file_handle);
    if ( !src_file )
    {
        vmi_free_unicode_str(dst_file_name_us);
        return;
    }

    char* dst_file_p = NULL;
    if (dst_file_root_handle)
    {
        char* dst_file_root = drakvuf_get_filename_from_handle(drakvuf, info, dst_file_root_handle);
        dst_file_p = g_strdup_printf("%s\\%s", dst_file_root ?: "", dst_file_name_us->contents);
        g_free(dst_file_root);
    }
    else if (is_absolute_path(reinterpret_cast<char*>(dst_file_name_us->contents)))
    {
        dst_file_p = g_strdup(reinterpret_cast<char*>(dst_file_name_us->contents));
    }
    else
    {
        char* dst_file_root_p = get_parent_folder(src_file);
        dst_file_p = g_strdup_printf("%s\\%s", dst_file_root_p ?: "", dst_file_name_us->contents);
        g_free(dst_file_root_p);
    }
    vmi_free_unicode_str(dst_file_name_us);

    switch (f->format)
    {
        case OUTPUT_CSV:
            printf("filetracer," FORMAT_TIMEVAL ",%" PRIu32 ",0x%" PRIx64 ",\"%s\",%" PRIi64",%s,%s,%s,%s\n",
                   UNPACK_TIMEVAL(info->timestamp), info->vcpu, info->regs->cr3, info->proc_data.name, info->proc_data.userid,
                   syscall_name, operation_name, src_file, dst_file_p);
            break;

        case OUTPUT_KV:
            printf("filetracer Time=" FORMAT_TIMEVAL ",PID=%d,PPID=%d,ProcessName=\"%s\",Method=%s,Operation=%s,FileSrc=\"%s\",FileDst=\"%s\"\n",
                   UNPACK_TIMEVAL(info->timestamp), info->proc_data.pid, info->proc_data.ppid, info->proc_data.name,
                   syscall_name, operation_name,
                   src_file, dst_file_p);
            break;

        default:
        case OUTPUT_DEFAULT:
            printf("[FILETRACER] TIME:" FORMAT_TIMEVAL " VCPU:%" PRIu32 " CR3:0x%" PRIx64 ",\"%s\" %s:%" PRIi64 " %s,%s,%s,%s\n",
                   UNPACK_TIMEVAL(info->timestamp), info->vcpu, info->regs->cr3, info->proc_data.name, USERIDSTR(drakvuf), info->proc_data.userid,
                   syscall_name, operation_name, src_file, dst_file_p);
            break;
    }

    g_free(dst_file_p);
    g_free(src_file);
}

static event_response_t cb(drakvuf_t drakvuf, drakvuf_trap_info_t* info)
{
    addr_t attr = drakvuf_get_function_argument(drakvuf, info, 3);
    objattr_read(drakvuf, info, attr);
    return 0;
}

static event_response_t cb2(drakvuf_t drakvuf, drakvuf_trap_info_t* info)
{
    addr_t attr = drakvuf_get_function_argument(drakvuf, info, 1);
    objattr_read(drakvuf, info, attr);
    return 0;
}

#define FILE_RENAME_INFORMATION 10

static event_response_t setinformation_cb(drakvuf_t drakvuf, drakvuf_trap_info_t* info)
{
    addr_t handle = drakvuf_get_function_argument(drakvuf, info, 1);
    addr_t fileinfo = drakvuf_get_function_argument(drakvuf, info, 3);
    uint32_t fileinfoclass = (uint32_t)drakvuf_get_function_argument(drakvuf, info, 5);

    if (fileinfoclass == FILE_RENAME_INFORMATION)
    {
        vmi_instance_t vmi = drakvuf_lock_and_get_vmi(drakvuf);
        print_rename_file_info(vmi, drakvuf, info, handle, fileinfo);
        drakvuf_release_vmi(drakvuf);
    }

    return 0;
}

/* ----------------------------------------------------- */

static void register_trap( drakvuf_t drakvuf, const char* syscall_name,
                           drakvuf_trap_t* trap,
                           event_response_t(*hook_cb)( drakvuf_t drakvuf, drakvuf_trap_info_t* info ) )
{
    if ( !drakvuf_get_function_rva( drakvuf, syscall_name, &trap->breakpoint.rva) ) throw -1;

    trap->name = syscall_name;
    trap->cb   = hook_cb;

    if ( ! drakvuf_add_trap( drakvuf, trap ) ) throw -1;
}

filetracer::filetracer(drakvuf_t drakvuf, const void* config, output_format_t output)
{
    int addr_size = drakvuf_get_address_width(drakvuf); // 4 or 8 (bytes)
    this->format = output;

    if ( !drakvuf_get_struct_member_rva(drakvuf, "_OBJECT_ATTRIBUTES", "ObjectName", &this->objattr_name) )
        throw -1;
    if ( !drakvuf_get_struct_member_rva(drakvuf, "_OBJECT_ATTRIBUTES", "RootDirectory", &this->objattr_root) )
        throw -1;
    if ( !drakvuf_get_struct_member_rva(drakvuf, "_OBJECT_ATTRIBUTES", "Attributes", &this->objattr_attr) )
        throw -1;
    // Offset of the RootDirectory field in _FILE_RENAME_INFORMATION structure
    this->newfile_root_offset = addr_size;
    // Offset of the FileName field in _FILE_RENAME_INFORMATION structure
    this->newfile_name_offset = addr_size * 2 + 4;
    // Offset of the FileNameLength field in _FILE_RENAME_INFORMATION structure
    this->newfile_name_length_offset = addr_size * 2;

    assert(sizeof(trap)/sizeof(trap[0]) > 9);
    register_trap(drakvuf, "NtCreateFile",          &trap[0], cb);
    register_trap(drakvuf, "ZwCreateFile",          &trap[1], cb);
    register_trap(drakvuf, "NtOpenFile",            &trap[2], cb);
    register_trap(drakvuf, "ZwOpenFile",            &trap[3], cb);
    register_trap(drakvuf, "NtOpenDirectoryObject", &trap[4], cb);
    register_trap(drakvuf, "ZwOpenDirectoryObject", &trap[5], cb);
    register_trap(drakvuf, "NtQueryAttributesFile", &trap[6], cb2);
    register_trap(drakvuf, "ZwQueryAttributesFile", &trap[7], cb2);
    register_trap(drakvuf, "NtSetInformationFile",  &trap[8], setinformation_cb);
    register_trap(drakvuf, "ZwSetInformationFile",  &trap[9], setinformation_cb);
}

filetracer::~filetracer()
{
}
