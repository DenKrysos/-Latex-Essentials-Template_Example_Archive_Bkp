void* ¢\color{eclipse_keyword2}\bfseries active\_query\_ach¢(void* args){
	if(net->client_active){
		char* ser_packet = NULL;
		int packLen = ¢\color{eclipse_keyword2}make\_AreYouHub\_packet¢(ser_packet);
		//find first Client with lower MAC Address in LLDP table and try querying it
		¢\color{eclipse_keyword1}discovery\_device\_t¢ current;
		if(net->this_client->next != NULL){
			current = net->this_client->next;
		}else{
			current = net->lldp_list;
		}
		while(current != net->this_client){
			if(¢\color{eclipse_keyword2}try\_send\_packet¢(current, ser_packet, packLen)){
				break;
			}
			if(current->next != NULL){
				current = current->next;
			}else{
				//go back to the first node
				current = net->lldp_list;
			}
		}
		free(ser_packet);
	}else{
		//reactivate Client if it does not receive a query for a certain amount of cycles
		if(net->cycle >= CYCLES_BEFORE_REACTIVATION){
			net->cycle_counter = 0;
			net->client_active = 1;
		}else{
			net->cycle_counter++;
		}
	}
}

void ¢\color{eclipse_keyword2}\bfseries received\_packet\_handler\_ac¢(void* arg, const char* data, const int data_len, struct in_addr sender_ip){
	¢\color{eclipse_keyword1}discovery\_device\_t¢ sender_dev = ¢\color{eclipse_keyword2}get\_device\_by\_ip¢(net->lldp_list, sender_ip);
	¢\color{eclipse_keyword1}discovery\_packet¢ packet = ¢\color{eclipse_keyword2}deserialize\_packet¢t(data, &packet);
	switch(¢\color{eclipse_keyword2}get\_packet\_type¢(packet)){
	case ARE_YOU_HUB:{
		if(*net->Hub_found){
			char *ser_packet = NULL;
			int packLen = make\_NotHubButIKnowIt\_packet(&serialized_packet, *net->hub_addr);
			¢\color{eclipse_keyword2}try\_send\_packet¢(sender_dev, ser_packet, packLen);
			free(&ser_packet);
			break;
		}
		if(net->client_active){
			if(¢\color{eclipse_keyword2}compare\_devices¢(net->this_client, sender_dev)==1){
				net->saved_client = sender_dev;
			}else{
				net->client_active = 0;
				net->cycle_counter = 0;
				net->saved_client = sender_dev;
			}
		}else{
			if(¢\color{eclipse_keyword2}compare\_devices¢(net->this_client, sender_dev)==1){
				net->cycle_counter = 0;
				net->saved_client = sender_dev;
			}
		}
		//forward the packet to the next Client on the list
		//packet doesn't have to be created and can be forwarded as is
		¢\color{eclipse_keyword1}discovery\_device\_t¢ current;
		if(net->this_client->next != NULL){
			current = net->this_client->next;
		}else{
			current = net->lldp_list;
		}
		while(current != net->this_client){
			if(¢\color{eclipse_keyword2}try\_send\_packet¢(current, ser_packet, packLen)){
				break;
			}
			if(current->next != NULL){
				current = current->next;
			}else{
				//go back to the first node
				current = net->lldp_list;
			}
		}
		break;
	}
	case I_AM_HUB:{
		//store the received Hub IP
		*net->hub_addr = ¢\color{eclipse_keyword2}get\_hub\_addr¢(packet);
		//mark the sender as the Hub
		sender_dev->is_hub = 1;
		*net->Hub_found = 1;
		net->client_active = 0;
		char* ser_packet = NULL;
		int packLen = ¢\color{eclipse_keyword2}make\_NotHubButIKnowIt\_packet¢(&ser_packet, *net->hub_addr);
		//forward the Hub Addr to the saved Client
		if(net->saved_client != NULL){
			char *ser_packet = NULL;
			int packLen = ¢\color{eclipse_keyword2}make\_NotHubButIKnowIt\_packet¢(&ser_packet, *net->hub_addr);
			¢\color{eclipse_keyword2}try\_send\_packet¢(net->saved_client, ser_packet, packLen);
			free(ser_packet);
			net->saved_client = NULL;
		}
		break;
	}
	case NOT_HUB_BUT_I_KNOW_IT:{
		//store the received Hub IP
		*net->hub_addr = ¢\color{eclipse_keyword2}get\_hub\_addr¢(packet);
		//get the Hub from the neighbors list and mark it as the Hub
		hub_dev=¢\color{eclipse_keyword2}get\_device\_by\_ip¢(net->lldp_list, *net->hub_addr);
		hub_dev->is_hub = 1;
		*net->Hub_found = 1;
		//forward the Hub Addr to the saved Client
		//no new packet has to be created
		//received packet from previous Client can be forwarded to next one as is
		if(net->saved_client != NULL){
			¢\color{eclipse_keyword2}try\_send\_packet¢(net->saved_client, data, data_len);
			net->saved_client = NULL;
		}
		break;
	}
	}//end switch (type)
	¢\color{eclipse_keyword2}denkrv\_free\_packet¢(&recieved_packet);
	return;
}
