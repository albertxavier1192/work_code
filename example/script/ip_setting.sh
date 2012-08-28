#!/bin/sh
WIRED_INTERFACE=eth1
CONF_DEV=/dev/sda2
CONF_DIR=/utm/conf
IP_SETTING=$CONF_DIR/script/ip_setting.sh
IP_SETTING_TEMP=$CONF_DIR/script/ip_setting.sh_temp

mask2cidr() {
    local nbits=0
    OFIS=$IFS
    IFS=.
    for dec in $1 ; do
        case $dec in
            255) let nbits+=8;;
            254) let nbits+=7;;
            252) let nbits+=6;;
            248) let nbits+=5;;
            240) let nbits+=4;;
            224) let nbits+=3;;
            192) let nbits+=2;;
            128) let nbits+=1;;
            0);;
	    *)nbits=-1; break;;
	esac
    done
    IFS=$OIFS

    if [ $nbits -eq -1 ]; then
	echo "Invaild valid"; exit
    fi
    return $nbits
}

valid_ip()
{
    local temp=`echo $1 | grep "[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}"`
    if [ -z $temp ]; then
	echo "Invaild value"; exit
    fi
}

#main
echo "Sensor IP : " | tr -d '\n'
read input
valid_ip $input
sensor_ip=$input

echo "Netmask : " | tr -d '\n'
read input
valid_ip $input
sensor_netmask=$input
mask2cidr $sensor_netmask
bit_netmask=$?

echo "Gateway : " | tr -d '\n'
read input
valid_ip $input
sensor_gw=$input

mount $CONF_DEV $CONF_DIR > /dev/null 2>&1

#delete ip_setting.sh
sed "/ip addr add .* dev $WIRED_INTERFACE/d" $IP_SETTING > $IP_SETTING_TEMP
mv $IP_SETTING_TEMP $IP_SETTING
sed "/ip route add default via .* dev $WIRED_INTERFACE/d" $IP_SETTING > $IP_SETTING_TEMP
mv $IP_SETTING_TEMP $IP_SETTING

#add ip_setting.sh
echo "ip addr add $sensor_ip/$bit_netmask dev $WIRED_INTERFACE" >> $IP_SETTING
echo "ip route add default via $sensor_gw dev $WIRED_INTERFACE" >> $IP_SETTING

umount $CONF_DIR > /dev/null 2>&1

#set ip
ip addr add $sensor_ip/$bit_netmask dev $WIRED_INTERFACE
ip route add default via $sensor_gw dev $WIRED_INTERFACE
