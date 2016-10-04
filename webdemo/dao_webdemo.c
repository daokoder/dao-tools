
#include "dao.h"

extern int DaoBigint_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoBinary_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoCoroutine_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoCrypto_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoDataframe_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoMath_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoMeta_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoSerializer_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoStatistics_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoFormat_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoScanner_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoTemplate_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoTime_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoHtml_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoJson_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoImage_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoCanvas_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoZip_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );

DaoVirtualModule dao_virtual_modules[] =
{
  { "$(CMD_DIR)/lib/dao/modules/libdao_bigint.so", 0, NULL, DaoBigint_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_binary.so", 0, NULL, DaoBinary_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_coroutine.so", 0, NULL, DaoCoroutine_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_crypto.so", 0, NULL, DaoCrypto_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_dataframe.so", 0, NULL, DaoDataframe_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_math.so", 0, NULL, DaoMath_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_meta.so", 0, NULL, DaoMeta_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_serializer.so", 0, NULL, DaoSerializer_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_statistics.so", 0, NULL, DaoStatistics_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/string/libdao_format.so", 0, NULL, DaoFormat_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/string/libdao_scanner.so", 0, NULL, DaoScanner_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/string/libdao_template.so", 0, NULL, DaoTemplate_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_time.so", 0, NULL, DaoTime_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/web/libdao_html.so", 0, NULL, DaoHtml_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/web/libdao_json.so", 0, NULL, DaoJson_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_image.so", 0, NULL, DaoImage_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_canvas.so", 0, NULL, DaoCanvas_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_zip.so", 0, NULL, DaoZip_OnLoad },
  { NULL, 0, NULL, NULL }
};

void DaoWebDemo_AddModules( DaoVmSpace *vmspace )
{
	DaoVmSpace_AddVirtualModules( vmspace, dao_virtual_modules );
}
