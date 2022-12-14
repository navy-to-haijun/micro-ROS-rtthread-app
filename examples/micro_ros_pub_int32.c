
#include <rtthread.h>
#include <micro_ros_rtt.h>
#include <stdio.h>
#if defined  MICROS_EXAMPLE_PUB_INT32 

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

static rcl_publisher_t publisher;
static std_msgs__msg__Int32 msg;

static rclc_executor_t executor;
static rclc_support_t support;
static rcl_allocator_t allocator;

static rcl_node_t node;
static rcl_timer_t timer;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){rt_kprintf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); return;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){rt_kprintf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

static void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
    // RCLC_UNUSED(last_call_time);
    if (timer != NULL) 
    {
        RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
        msg.data++;
    }
    else {
        rt_kprintf("[micro_ros] timer null\n");
    }
}

static void microros_pub_int32_thread_entry(void *parameter)
{
    while(1)
    {
        rt_thread_mdelay(100);
        RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
    }
}

static void microros_pub_int32(int argc, char* argv[])
{
    #if defined MICROROS_SERIAL
    set_microros_transports();
    #endif

    #if defined MICROROS_UDP
    set_microros_udp_transports(MICROROS_IP, MICROROS_PORT);
    #endif
    allocator = rcl_get_default_allocator();

    //create init_options
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    // create node
    RCCHECK(rclc_node_init_default(&node, "micro_ros_rtt_node", "", &support));
    // create publisher
    RCCHECK(rclc_publisher_init_default(
      &publisher,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "micro_ros_rtt_node_publisher")
      );
    // create timer
    const unsigned int timer_timeout = 1000;
    RCCHECK(rclc_timer_init_default(
      &timer,
      &support,
      RCL_MS_TO_NS(timer_timeout),
      timer_callback)
      );
    // create executor
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_timer(&executor, &timer));

    msg.data = 0;
    rt_kprintf("[micro_ros] micro_ros init successful.\n");
    rt_thread_t thread = rt_thread_create("mr_pubint32", microros_pub_int32_thread_entry, RT_NULL, 2048, 25, 10);
    if(thread != RT_NULL)
    {
        rt_thread_startup(thread);
        rt_kprintf("[micro_ros] New thread mr_pubint32\n");
    }
    else
    {
        rt_kprintf("[micro_ros] Failed to create thread mr_pubint32\n");
    }
}
MSH_CMD_EXPORT(microros_pub_int32, microros publish int32 example)

#endif // MICRO_ROS_USE_SERIAL