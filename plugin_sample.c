#include <vnet/plugin/plugin.h>
#include <plugin_sample/plugin_sample.h>

plugin_sample_main_t plugin_sample_main;

//开关实现
    static int
plugin_sample_base_enable_disable(u32 sw_if_index, //index
        int enable_disable,
        const char *plug_node_name,
        const char *plugin_name)
{
    vnet_sw_interface_t *sw;
    int ret = 0;

    /* Utterly wrong? */
    //vnet_main结构中的interface_main结构中的sw接口
    if (pool_is_free_index (plugin_sample_main.vnet_main->interface_main.sw_interfaces,
                sw_if_index)) //接口索引
        return VNET_API_ERROR_INVALID_SW_IF_INDEX;

    /* Not a physical port? */
    sw = vnet_get_sw_interface(plugin_sample_main.vnet_main,        //vnet_main结构
            sw_if_index);
    if (sw->type != VNET_SW_INTERFACE_TYPE_HARDWARE)
        return VNET_API_ERROR_INVALID_SW_IF_INDEX;

    vnet_feature_enable_disable(plug_node_name,
            plugin_name,
            sw_if_index,
            enable_disable, 0, 0);

    return ret;
}

    static clib_error_t*
plugin_sample_enable_disable_command_fn(vlib_main_t* vm,        //vlib_main结构
        unformat_input_t *input,
        vlib_cli_command_t *cmd)
{
    u32 sw_if_index = ~0;       //~0 取反全为1
    int enable_disable = 1;

    while(unformat_check_input(input) != UNFORMAT_END_OF_INPUT) //非空则继续输入
    {
        if (unformat(input, "enable"))
            enable_disable = 1;
        else if (unformat(input, "disable"))
            enable_disable = 0;
        else if (unformat(input, "%U",
                    unformat_vnet_sw_interface,
                    plugin_sample_main.vnet_main, &sw_if_index));
        else
            break;
    }

    if (sw_if_index == ~0)
        return clib_error_return(0, "Please specify an interface...");

    //调用plugin_sample_base_enable_disable()
    plugin_sample_base_enable_disable(sw_if_index,
            enable_disable,
            "ip4-unicast", //挂载节点
            "plugin_sample");

    return 0;
}
    static clib_error_t*
plugin_sample6_enable_disable_command_fn(vlib_main_t* vm,       //vlib_main结构
        unformat_input_t *input,
        vlib_cli_command_t *cmd)
{
    u32 sw_if_index = ~0;       //~0 取反全为1
    int enable_disable = 0; // 默认为关闭状态

    while(unformat_check_input(input) != UNFORMAT_END_OF_INPUT) //非空则继续输入
    {
        if (unformat(input, "enable"))
            enable_disable = 1;
        else if (unformat(input, "disable"))
            enable_disable = 0;
        else if (unformat(input, "%U",
                    unformat_vnet_sw_interface,
                    plugin_sample_main.vnet_main, &sw_if_index));
        else
            break;
    }

    if (sw_if_index == ~0)
        return clib_error_return(0, "Please specify an interface...");

    //调用plugin_sample_base_enable_disable()
    plugin_sample_base_enable_disable(sw_if_index,
            enable_disable,
            "ip6-unicast", //挂载节点
            "plugin_sample6");

    return 0;
}

//注册开关CLI
//指定interface的开关
VLIB_CLI_COMMAND (plugin_sample_command, static) = {
    .path = "plugin sample",
    .short_help =
        "plugin sample <interface-name> [disable]",
    .function = plugin_sample_enable_disable_command_fn,
};
VLIB_CLI_COMMAND (plugin_sample6_command, static) = {
    .path = "plugin sample6",
    .short_help =
        "plugin sample <interface-name> [enable | disable]",
    .function = plugin_sample6_enable_disable_command_fn,
};

/*注册插件*****start*****/
VLIB_PLUGIN_REGISTER () = {
    .version = PLUGIN_SAMPLE_PLUGIN_BUILD_VER,
    .description = "Print IPv4/IPv6 Header",
};

static clib_error_t *plugin_sample_init(vlib_main_t* vm)
{
    plugin_sample_main.vnet_main = vnet_get_main();
    return 0;
}

VLIB_INIT_FUNCTION(plugin_sample_init);
/*注册插件*****end*****/


//将node注册在ip4-unicast的arc中，指定ip-lookup之前Hook到数据包
VNET_FEATURE_INIT(plugin_sample, static) =
{
    .arc_name = "ip4-unicast",
    .node_name = "plugin_sample",
    .runs_before = VNET_FEATURES("ip4-lookup"),
};
VNET_FEATURE_INIT(plugin_sample6, static) =
{
    .arc_name = "ip6-unicast",
    .node_name = "plugin_sample6",
    .runs_before = VNET_FEATURES("ip6-lookup"),
};
