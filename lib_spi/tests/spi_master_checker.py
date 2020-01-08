import xmostest

class SPIMasterChecker(xmostest.SimThread):
    """"
    This simulator thread will act as SPI slave and check any transactions
    caused by the master.
    """
    def __init__(self, sck_port, mosi_port, miso_port, ss_ports, setup_strobe_port, setup_data_port):
        self._miso_port = miso_port
        self._mosi_port = mosi_port
        self._sck_port = sck_port
        self._ss_ports = ss_ports
        self._setup_strobe_port = setup_strobe_port
        self._setup_data_port = setup_data_port

    def get_setup_data(self, xsi, setup_strobe_port, setup_data_port):
        self.wait_for_port_pins_change([setup_strobe_port])
        self.wait_for_port_pins_change([setup_strobe_port])
        return xsi.sample_port_pins(setup_data_port)

    def run(self):
        xsi = self.xsi

        sck_value = xsi.sample_port_pins(self._sck_port)
        ss_value = []

	for i in range(len(self._ss_ports)):
          ss_value.append(xsi.sample_port_pins(self._ss_ports[i]))

        print "SPI Master checker started"
        while True:
            #first do the setup rx
            strobe_val = xsi.sample_port_pins(self._setup_strobe_port)
	    if strobe_val == 1:
              self.wait_for_port_pins_change([self._setup_strobe_port])

            expected_cpol = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_cpha = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_frequency_in_khz = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_mosi_enabled = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_miso_enabled = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_device_id = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_interframe_space = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)
            expected_num_bytes = self.get_setup_data(xsi, self._setup_strobe_port, self._setup_data_port)

            clock_half_period = 1000000/(expected_frequency_in_khz*2)

            all_ss_deserted = True
            for i in range(len(self._ss_ports)):
               if (xsi.sample_port_pins(self._ss_ports[i]) == 0):
		 all_ss_deserted = False
                 break

            while not all_ss_deserted:
              self.wait_for_port_pins_change(self._ss_ports)
              all_ss_deserted = True
              for i in range(len(self._ss_ports)):
                all_ss_deserted = all_ss_deserted and (xsi.sample_port_pins(self._ss_ports[i]) == 1)

            error = False

            active_slave = -1

            while(active_slave == -1):
              self.wait_for_port_pins_change(self._ss_ports)

              for i in range(len(self._ss_ports)):
                if xsi.sample_port_pins(self._ss_ports[i]) == 0:
                  active_slave = i
                  break


            last_clock_event_time = xsi.get_time();

            rx_bit_counter = 0
            tx_bit_counter = 0
            tx_data = [0xfe, 0xf7, 0xfb, 0xef, 0xdf, 0xbf, 0xfd, 0x7f, 0x01, 0x08, 0x04, 0x10, 0x20, 0x04, 0x02, 0x80]
            rx_data = [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x04, 0x80, 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f]
            rx_byte = 0
            tx_byte = tx_data[0]
            # check the polarity
            if xsi.sample_port_pins(self._sck_port) != expected_cpol:
              print("ERROR: unexpected clock polarity at the slave select point")
              error = True
            clock_edge_number = 0

            #probably not physically possible but good for testing
            if expected_cpha == 0:
              xsi.drive_port_pins(self._miso_port, (tx_byte>>7)&1)
              tx_bit_counter += 1
              tx_byte = tx_byte << 1

            ss_value = xsi.sample_port_pins(self._ss_ports[active_slave])
            sck_value = xsi.sample_port_pins(self._sck_port)

            while ss_value == 0:
              self.wait_for_port_pins_change(self._ss_ports + [self._sck_port])

              for i in range(len(self._ss_ports)):
                if i != active_slave and xsi.sample_port_pins(self._ss_ports[i]) == 0:
                  error = True
                  print "Second slave selected during first transaction"

              if (ss_value == xsi.sample_port_pins(self._ss_ports[active_slave]) and (sck_value == xsi.sample_port_pins(self._sck_port))):
                continue

              ss_value = xsi.sample_port_pins(self._ss_ports[active_slave])
              sck_value = xsi.sample_port_pins(self._sck_port)

              if ss_value == 0:
                clock_event_time = xsi.get_time();
                measured_time_elapsed = clock_event_time - last_clock_event_time
                if clock_edge_number > 1 and (measured_time_elapsed*1.05) < clock_half_period :
                  print("ERROR: Clock half period less than allowed for given SCLK frequency" )
                  print("%d %d " % (measured_time_elapsed, clock_half_period))
                  error = True
                last_clock_event_time =clock_event_time

              #check that the clock edges never go faster than the expected clock rate
              if ss_value == 0:
                clock_edge_number += 1
               #print clock_edge_number
                #the the clock must have transitioned
                if sck_value == (expected_cpha ^ expected_cpol):
                  if expected_miso_enabled == 1:
                    #drive data out
                    xsi.drive_port_pins(self._miso_port, (tx_byte>>7)&1)
                    tx_bit_counter += 1
                    tx_byte = tx_byte << 1
                    if (tx_bit_counter%8) == 0:
                      index = tx_bit_counter/8
                      if index < 16:
                        tx_byte = tx_data[tx_bit_counter/8]
                      else:
                        tx_byte = 0;

                else:
                  #clock data in
                  if expected_mosi_enabled == 1:
                    rx_byte = rx_byte << 1
                    rx_byte += xsi.sample_port_pins(self._mosi_port)
                    rx_bit_counter = rx_bit_counter + 1
                    if((rx_bit_counter%8) == 0):
                      expected_rx_byte = rx_data[(rx_bit_counter/8) - 1]
                      #print "slave got {seen} and expected {expect}".format(seen=rx_byte, expect=expected_rx_byte)
                      if expected_rx_byte != rx_byte:
                        print "ERROR: slave recieved incorrect data Got:%02x Expected:%02x"%(rx_byte, expected_rx_byte)
                        error = True
                      rx_byte = 0
              else:
                if clock_edge_number != expected_num_bytes*2*8:
                  error = True
                  print "ERROR: incorrect number of clock edges at slave {seen}/{expect}".format(seen=clock_edge_number, expect=expected_num_bytes*2*8)
                if error:
                  print "Fail: CPOL:{cpol} CPHA:{cpha} KHz:{freq} MOSI Enabled:{mosi_enabled} MISO Enabled:{miso_enabled}".format(
			cpol=expected_cpol, cpha=expected_cpha,
                        mosi_enabled = expected_mosi_enabled, miso_enabled = expected_miso_enabled,
                        freq=expected_frequency_in_khz)
