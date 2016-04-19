#include "tracewrap.h"
#include "trace_consts.h"

#include <err.h>
#include "qemu/log.h"

static Frame * g_frame;
static uint64_t frames_per_toc_entry = 64LL;
static uint32_t open_frame = 0;
static FILE *file = NULL;

/* don't use the following data directly!
   use toc_init, toc_update and toc_write functions instead */
static uint64_t *toc = NULL;
static int toc_entries = 0;
static int toc_capacity = 0;
static uint64_t toc_num_frames = 0;

#define WRITE(x) do {                                       \
        if (!file)                                          \
            err(1, "qemu_trace is not initialized");        \
        if (fwrite(&(x), sizeof(x),1,file) != 1)            \
            err(1, "fwrite failed");                        \
    } while(0)

#define SEEK(off) do {                          \
    if (fseek(file,(off), SEEK_SET) < 0)        \
        err(1, "stream not seekable");          \
    } while(0)


static void toc_init(void) {
    if (toc_entries != 0)
        err(1, "qemu_trace was initialized twice");
    toc = g_new(uint64_t, 1024);
    toc_capacity = 1024;
    toc_entries = 0;
}

static void toc_append(uint64_t entry) {
    if (toc_capacity <= toc_entries) {
        toc = g_renew(uint64_t, toc, toc_capacity * 2);
        toc_capacity *= 2;
    }
    toc[toc_entries++] = entry;
}

static void toc_write(void) {
    int64_t toc_offset = ftell(file);
    if (toc_offset > 0) {
        int i = 0;
        WRITE(frames_per_toc_entry);
        for (i = 0; i < toc_entries; i++)
            WRITE(toc[i]);
        SEEK(num_trace_frames_offset);
        WRITE(toc_num_frames);
        SEEK(toc_offset_offset);
        WRITE(toc_offset);
    }
}

static void toc_update(void) {
    toc_num_frames++;
    if (toc_num_frames % frames_per_toc_entry == 0) {
        int64_t off = ftell(file);
        if (off >= 0) toc_append(off);
    }
}

static void init_header(void) {
    uint64_t toc_off = 0L;
    WRITE(magic_number);
    WRITE(out_trace_version);
    WRITE(bfd_arch);
    WRITE(bfd_machine);
    WRITE(toc_num_frames);
    WRITE(toc_off);
}

void qemu_trace_init(const char *name, int argc, char **argv) {
    qemu_log("Initializing tracer\n");
    name = name ? g_strdup(name) : g_strdup_printf("%s.frames", basename(argv[0]));
    file = fopen(name, "wb");
    if (file == NULL)
        err(1, "tracewrap: can't open trace file %s", name);
    init_header();
    toc_init();
}

void qemu_trace_newframe(target_ulong addr, int thread_id) {
    if (open_frame) {
        qemu_log("frame is still open");
        qemu_trace_endframe(NULL, 0, 0);
    }

    open_frame = 1;
    thread_id = 1;
    g_frame = g_new(Frame,1);
    frame__init(g_frame);

    StdFrame *sframe = g_new(StdFrame, 1);
    std_frame__init(sframe);
    g_frame->std_frame = sframe;

    sframe->address = addr;
    sframe->thread_id = thread_id;

    OperandValueList *ol_in = g_new(OperandValueList,1);
    operand_value_list__init(ol_in);
    ol_in->n_elem = 0;
    sframe->operand_pre_list = ol_in;

    OperandValueList *ol_out = g_new(OperandValueList,1);
    operand_value_list__init(ol_out);
    ol_out->n_elem = 0;
    sframe->operand_post_list = ol_out;
}

static inline void free_operand(OperandInfo *oi) {
    OperandInfoSpecific *ois = oi->operand_info_specific;

    //Free reg-operand
    RegOperand *ro = ois->reg_operand;
    if (ro && ro->name)
        g_free(ro->name);
    g_free(ro);

    //Free mem-operand
    MemOperand *mo = ois->mem_operand;
    g_free(mo);
    g_free(oi->value.data);
    g_free(oi->taint_info);
    g_free(ois);
    g_free(oi->operand_usage);
    g_free(oi);
}

void qemu_trace_add_operand(OperandInfo *oi, int inout) {
    if (!open_frame) {
        if (oi)
            free_operand(oi);
        return;
    }
    OperandValueList *ol;
    if (inout & 0x1) {
        ol = g_frame->std_frame->operand_pre_list;
    } else {
        ol = g_frame->std_frame->operand_post_list;
    }

    oi->taint_info = g_new(TaintInfo, 1);
    taint_info__init(oi->taint_info);
    oi->taint_info->no_taint = 1;
    oi->taint_info->has_no_taint = 1;

    ol->n_elem += 1;
    ol->elem = g_renew(OperandInfo *, ol->elem, ol->n_elem);
    ol->elem[ol->n_elem - 1] = oi;
}

void qemu_trace_endframe(CPUArchState *env, target_ulong pc, target_ulong size) {
    int i = 0;
    StdFrame *sframe = g_frame->std_frame;

    if (!open_frame) return;

    sframe->rawbytes.len = size;
    sframe->rawbytes.data = g_malloc(size);
    for (i = 0; i < size; i++) {
        sframe->rawbytes.data[i] = cpu_ldub_code(env, pc+i);
    }

    size_t msg_size = frame__get_packed_size(g_frame);
    uint8_t *packed_buffer = g_alloca(msg_size);
    uint64_t packed_size = frame__pack(g_frame, packed_buffer);
    WRITE(packed_size);
    if (fwrite(packed_buffer, packed_size, 1, file) != 1)
        err(1, "failed to write a frame");

    toc_update();

    //counting num_frames in newframe does not work by far ...
    //how comes? disas_arm_insn might not always return at the end?
    for (i = 0; i < sframe->operand_pre_list->n_elem; i++)
        free_operand(sframe->operand_pre_list->elem[i]);
    g_free(sframe->operand_pre_list->elem);
    g_free(sframe->operand_pre_list);

    for (i = 0; i < sframe->operand_post_list->n_elem; i++)
        free_operand(sframe->operand_post_list->elem[i]);
    g_free(sframe->operand_post_list->elem);
    g_free(sframe->operand_post_list);

    g_free(sframe->rawbytes.data);
    g_free(sframe);
    g_free(g_frame);
    open_frame = 0;
}

void qemu_trace_finish(uint32_t exit_code) {
    toc_write();
    fclose(file);
}
