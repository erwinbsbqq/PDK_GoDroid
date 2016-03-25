#ifndef _ALI_GPIO_COMMON_H_
#define _ALI_GPIO_COMMON_H_




struct gpio_info {
    unsigned int gpio;
    unsigned int status;
};
#define GPIO_MAGIC                                      'G'

#define GPIO_REQUEST                                  _IOR(GPIO_MAGIC, 0, int)
#define GPIO_RELEASE                                  _IOR(GPIO_MAGIC, 1, int)
#define GPIO_GET_STATUS                               _IOR(GPIO_MAGIC, 2, struct gpio_info)
#define GPIO_SET_STATUS                               _IOW(GPIO_MAGIC, 3, struct gpio_info)
#define GPIO_SET_DIR                                  _IOW(GPIO_MAGIC, 4, struct gpio_info)



#endif /* _ALI_GPIO_COMMON_H_ */

