
#include "dao.h"

extern int DaoBigint_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoBinary_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoCoroutine_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoDataframe_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoMacro_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoMath_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoMeta_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoSerializer_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );
extern int DaoTime_OnLoad( DaoVmSpace *vms, DaoNamespace *ns );

DaoVModule dao_virtual_modules[] =
{
  { "$(CMD_DIR)/lib/dao/modules/libdao_bigint.so", 0, NULL, DaoBigint_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_binary.so", 0, NULL, DaoBinary_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_coroutine.so", 0, NULL, DaoCoroutine_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_dataframe.so", 0, NULL, DaoDataframe_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_macro.so", 0, NULL, DaoMacro_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_math.so", 0, NULL, DaoMath_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_meta.so", 0, NULL, DaoMeta_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_serializer.so", 0, NULL, DaoSerializer_OnLoad },
  { "$(CMD_DIR)/lib/dao/modules/libdao_time.so", 0, NULL, DaoTime_OnLoad },
  { NULL, 0, NULL, NULL }
};

void DaoWebDemo_AddModules( DaoVmSpace *vmspace )
{
	DaoVmSpace_AddVirtualModules( vmspace, dao_virtual_modules );
}
