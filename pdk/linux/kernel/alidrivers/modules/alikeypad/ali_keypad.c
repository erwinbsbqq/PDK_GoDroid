/*
 *      Alitech Keypad Driver
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/gpio_keys.h>
#include <ali_gpio.h>
#include "ali_keypad.h"

#define DRV_VERSION     "1.0.0"
#define ALI_KEYPAD_DEV_NAME	"ali_keypad"

#define KEY_PRESSED        1
#define KEY_RELEASE        0

static int ali_keypad_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int ali_keypad_close(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations ali_keypad_fops = {
	.owner = THIS_MODULE,
	.open  = ali_keypad_open,
	.release = ali_keypad_close,
};

static struct miscdevice ali_keypad_misc = {
	.fops = &ali_keypad_fops,
	.name = ALI_KEYPAD_DEV_NAME,
	.minor = MISC_DYNAMIC_MINOR,
};

static irqreturn_t ali_keypad_isr(int irq, void *dev_id)
{
	int i, num;
	struct gpio_button *btn = NULL;
	struct keypad_drvdata *priv = (struct keypad_drvdata *)dev_id;

	num = priv->nbuttons;
	for (i = 0; i < num; i++) {
		btn = &priv->button[i];
		if (get_gpio_interrupt_status(btn->gpio) > 0) {
			clear_gpio_interrupt_status(btn->gpio);
			if (ali_gpio_get_value(btn->gpio) == 0) {
				input_report_key(priv->input, btn->code,
					KEY_PRESSED);
				input_report_key(priv->input, btn->code,
					KEY_RELEASE);
				input_sync(priv->input);

				return IRQ_HANDLED;
			}
		}
	}

	return IRQ_NONE;
}

static int __init ali_keypad_probe(struct platform_device *pdev)
{
	int i, retval = 0;
	struct input_dev *input = NULL;
	struct device *dev = &pdev->dev;
	struct keypad_drvdata *priv = NULL;
	const struct gpio_keys_platform_data *pdata = dev_get_platdata(dev);

	/* platform data checking */
	if (IS_ERR(pdata)) {
		/* TODO : get data from DT */
		retval = PTR_ERR(pdata);
		dev_err(dev, "platform data acquires failure (err : %d) !\n",
			retval);
		return retval;
	}

	/* driver data allocating */
	priv = devm_kzalloc(dev, sizeof(struct keypad_drvdata) +
		pdata->nbuttons * sizeof(struct gpio_button),
		GFP_KERNEL);
	if (IS_ERR(priv)) {
		retval = PTR_ERR(priv);
		dev_err(dev, "device memory allocates failure (err : %d) !\n",
			retval);
		return retval;
	}

	retval = misc_register(&ali_keypad_misc);
	if (retval < 0) {
		dev_err(dev, "volbar misc register failure (err : %d) !\n",
			retval);
		goto drv_free;
	}

	/* input sub device setting */
	input = input_allocate_device();
	if (IS_ERR(input)) {
		retval = PTR_ERR(input);
		dev_err(dev, "input device allocates failure (err : %d) !\n",
			retval);
		goto  misc_unreg;
	}
	priv->input = input;
	input->name = ALI_KEYPAD_DEV_NAME;
	input->phys = "ali_keypad/input0";
	input->id.bustype = BUS_HOST;
	input->id.vendor  = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;
	input->dev.parent = dev;

	__set_bit(EV_KEY, input->evbit);
	__set_bit(EV_REP, input->evbit);

	/* platform data recording */
	priv->nbuttons = pdata->nbuttons;
	for (i = 0; i < priv->nbuttons; i++) {
		struct gpio_button *btn = &priv->button[i];

		btn->gpio = pdata->buttons[i].gpio;
		btn->code = pdata->buttons[i].code;
		btn->type = pdata->buttons[i].type;
		__set_bit(btn->code & KEY_MAX, input->keybit);
	}

	retval = input_register_device(input);
	if (retval < 0) {
		dev_err(dev, "input device register failure (err : %d) !\n",
				retval);
		goto input_free;
	}

	/* gpio isr register */
	retval = platform_get_irq(pdev, 0);
	if (retval < 0) {
		dev_err(dev, "device irq number request failure (err : %d) !\n",
				retval);
		goto  input_unreg;
	}
	priv->irq = retval;
	retval = devm_request_irq(dev, priv->irq,
		(irq_handler_t)ali_keypad_isr,
		 IRQF_SHARED, ALI_KEYPAD_DEV_NAME, priv);
	if (retval) {
		dev_err(dev, "device irq handler register failure (err : %d) !\n",
			retval);
		goto input_unreg;
	}

	platform_set_drvdata(pdev, priv);
	/*input_set_drvdata(input, priv);*/
	/*mutex_init(&priv->disable_lock);*/

	return 0;

input_unreg:
	input_unregister_device(priv->input);
input_free:
	input_free_device(priv->input);
	priv->input = NULL;
misc_unreg:
	misc_deregister(&ali_keypad_misc);
drv_free:
	devm_kfree(dev, priv);
	priv = NULL;

	return retval;
}

static int ali_keypad_remove(struct platform_device *pdev)
{
	struct keypad_drvdata *priv = platform_get_drvdata(pdev);

	if (IS_ERR(priv))
		return PTR_ERR(priv);

	misc_deregister(&ali_keypad_misc);
	input_unregister_device(priv->input);
	input_free_device(priv->input);
	priv->input = NULL;

	devm_free_irq(&pdev->dev, priv->irq, priv);

	devm_kfree(&pdev->dev, priv);
	priv = NULL;

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver ali_keypad_driver = {
	.driver = {
		.name = ALI_KEYPAD_DEV_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ali_keypad_probe,
	.remove = __devexit_p(ali_keypad_remove),
};

static int __init ali_keypad_driver_init(void)
{
	return platform_driver_register(&ali_keypad_driver);
}

static void __exit ali_keypad_driver_exit(void)
{
	platform_driver_unregister(&ali_keypad_driver);
}

module_init(ali_keypad_driver_init);
module_exit(ali_keypad_driver_exit);

MODULE_DESCRIPTION("Alitech Keypad Driver");
MODULE_AUTHOR("Rivaille Zhu");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
