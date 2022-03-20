#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>
int main()
{
    srand(time(NULL));
    FILE *fp = fopen("./sys_config.txt", "r");
    char sys[1000][1000] = {0};
    int j = 0;
    if(NULL == fp)
    {
        // fprintf(fw,"failed to open\n");
    }
    while(!feof(fp))
    {
        memset(sys[j], 0, 1000);
        fgets(sys[j], 999, fp);
        // fprintf(fw,"%s", sys[j]);
        j++;
    }
    int tlb_policy = 0; //1 = LRU 2 = FIFO
    int page_policy = 0; //1 = Clock 2 = FIFO
    int frame_policy = 0; //1 = GLOBAL 2 = LOCAL
    int process_num = 0;
    int page_num = 0;
    int frame_num = 0;
    int tlb[32][3];
    for(int i = 0; i < 32; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            tlb[i][j] = -1;
        }
    }
    if (strcmp(sys[0],"TLB Replacement Policy: LRU\n") == 0)
    {
        tlb_policy = 1;
    }
    else
    {
        tlb_policy = 2;
    }
    if (strcmp(sys[1],"Page Replacement Policy: FIFO\n") == 0)
    {
        page_policy = 2;
    }
    else
    {
        page_policy = 1;
    }
    if (strcmp(sys[2],"Frame Allocation Policy: GLOBAL\n") == 0)
    {
        frame_policy = 1;
    }
    else
    {
        frame_policy = 2;
    }
    for(int i = 0; i < 3; i++)
    {
        char *p = strtok(sys[i+3],":");
        p = strtok(NULL,":");
        // fprintf(fw,"%d\n",atoi(p));
        if(i == 0)
        {
            process_num = atoi(p);
        }
        else if(i == 1)
        {
            page_num = atoi(p);
        }
        else if(i == 2)
        {
            frame_num = atoi(p);
        }
    }
    fclose(fp);
    // fprintf(fw,"TLB:%d\nPAGE:%d\nFRAME:%d\n",tlb_policy,page_policy,frame_policy);
    // fprintf(fw,"process_num:%d\nPage_numE:%d\nframe_num:%d\n",process_num,page_num,frame_num);
    int page_table[process_num][page_num][3];
    for (int k = 0; k < process_num; k++)
    {
        for(int i = 0; i < page_num; i++)
        {
            for(int j = 0; j < 3; j++)
            {
                page_table[k][i][j] = -1;
            }
            // page_table[k][i][2] = 0;
        }
    }
    int disk[page_num*process_num][2];
    for (int i = 0; i < page_num*process_num; i++)
    {
        for(int j = 0; j < 2; j++)
        {
            disk[i][j] = -1;
        }
    }
    int free_frame_list[frame_num];//0 = free 1 = not free
    int physical_memory_list[frame_num][2];
    int memory_localfifo[frame_num];
    for(int i = 0; i < frame_num; i++)
    {
        free_frame_list[i] = 0;
        memory_localfifo[i] = -1;
        // physical_memory_list = "";
    }
    FILE *fd = fopen("./trace.txt", "r");
    char trace[1000][1000] = {0};
    int k = 0;
    if(NULL == fd)
    {
        // fprintf(fw,"failed to open\n");
    }
    while(!feof(fd))
    {
        memset(trace[k], 0, 1000);
        fgets(trace[k], 999, fd);
        // fprintf(fw,"%s", trace[k]);
        k++;
    }
    // fprintf(fw,"Total\t%d\tReference\n",k);
    fclose(fd);
    //start reference
    int trace_time = k;
    char *parsed_first = strtok(trace[0], ",");
    parsed_first = strtok(NULL, ")");
    int address_first = atoi(parsed_first);
    char process_first = trace[0][10];
    int pro_int_first = process_first -= 65;
    char process_temp[10] = {0};
    process_temp[0] = trace[0][10];
    free_frame_list[0] = 1;
    page_table[pro_int_first][address_first][0] = 0;
    page_table[pro_int_first][address_first][2] = 1;
    char first_physical[10]  = {0};
    physical_memory_list[0][0] = pro_int_first;
    physical_memory_list[0][1] = address_first;
    // strcat(first_physical, process_temp);
    // strcat(first_physical, parsed_first);
    // strcat(physical_memory_list[0], first_physical);
    // physical_memory_list[0] = process_first;
    int physical_now = 1;

    FILE *fw = fopen("./trace_output.txt", "w+");
    fprintf(fw,"Process %c, TLB Miss, Page Fault, 0, Evict -1 of Process %c to -1, %d<<-1\n",trace[0][10],trace[0][10],address_first);
    tlb[0][0] = address_first;
    tlb[0][1] = 0;
    tlb[0][2] = 0;
    memory_localfifo[0] = 0;
    float count_fault[process_num][3];
    for (int i = 0; i < process_num; i++)
    {
        count_fault[i][0] = 0;
        count_fault[i][1] = 0;
        count_fault[i][2] = 0;
    }
    float hit_rate[process_num][2];
    for (int i = 0; i < process_num; i++)
    {
        hit_rate[i][0] = 0;
        hit_rate[i][1] = 0;
    }
    count_fault[pro_int_first][0] += 1;
    count_fault[pro_int_first][1] += 1;
    hit_rate[pro_int_first][0] +=2;
    hit_rate[pro_int_first][1] +=1;
    int last_process = pro_int_first;
    int now_process = -1;
    fprintf(fw,"Process %c, TLB Hit, %d=>0\n",trace[0][10],tlb[0][0]);
    int now_kick = 0;
    int local_kick[process_num];
    for (int i = 0; i < process_num; i++)
    {
        local_kick[i] = 0;
    }
    // printf("%f\n",hit_rate[0][0]);
    for(int i = 1; i < trace_time /*78*/; i++)
    {
        char *parsed = strtok(trace[i], ",");
        parsed = strtok(NULL, ")");
        int address = atoi(parsed);
        char process = trace[i][10];
        char temp[10] = {0};
        char physical[10]  = {0};
        temp[0] = trace[i][10];
        int pro_int = process -= 65;

        now_process = pro_int;
        count_fault[now_process][0] ++;
        hit_rate[now_process][0] ++;
        // int physical_now = 0;
        strcat(physical, temp);
        strcat(physical, parsed);
        // fprintf(fw,"%d\t%d\n",pro_int ,address);
        // fwrite(trace[i],1,strlen(trace[i]),fw);
        //Find TLB
        if(now_process != last_process) //clear tlb if process change
        {
            for(int m = 0; m < 32; m++)
            {
                for(int n = 0; n < 3; n++)
                {
                    tlb[m][n] = -1;
                }
            }
        }
        int tlb_hit = 0;
        for(int j = 0 ; j < 32; j++) //find in tlb
        {
            if (tlb[j][0] == address)
            {
                fprintf(fw,"Process %c, TLB Hit, %d=>%d\n",trace[i][10],address,tlb[j][1]);
                tlb[j][2] = 0;
                count_fault[now_process][2] ++;

                hit_rate[now_process][1] ++;
                for(int n = 0; n < 32; n++)
                {
                    if (n != j && tlb[n][1] != -1)
                    {
                        tlb[n][2] += 1;
                    }
                }
                tlb_hit = 1;
                page_table[pro_int][address][2] = 1;
                break;
            }
            else if (j == 31)
            {
                fprintf(fw,"Process %c, TLB Miss,",trace[i][10]);
                hit_rate[now_process][0] ++;
                hit_rate[now_process][1] ++;
            }
        }
        //find in page table
        int page_hit = -1;
        if (page_table[pro_int][address][0] != -1 && tlb_hit == 0)
        {
            page_hit = 1;
            fprintf(fw," Page Hit, %d=>%d",address,page_table[pro_int][address][0]);
            page_table[pro_int][address][2] = 1;
            //add to tlb
        }
        if(page_table[pro_int][address][0] == -1 && tlb_hit == 0)
        {
            count_fault[now_process][1] += 1;
            int change_disk_page = -1;
            int file_source = -1;
            fprintf(fw," Page Fault,");
            int frame = -1;
            for(int j = 0; j < frame_num; j++)
            {
                if (free_frame_list[j] == 0)
                {
                    frame = j;

                    page_table[pro_int][address][2] = 1;
                    break;
                }
            }
            if(frame == -1) //no free frame
            {
                if (page_policy == 2) //FIFO
                {
                    if(frame_policy == 1) //GLOBAL
                    {
                        //i %64
                        int change_frame = physical_now % frame_num;
                        frame = change_frame;
                        for(int m = 0; m < process_num*page_num; m++)
                        {
                            if(disk[m][1] == -1)
                            {
                                change_disk_page = m;
                                disk[m][0] = physical_memory_list[frame][0];
                                disk[m][1] = physical_memory_list[frame][1];
                                break;
                            }
                        }

                        // fprintf(fw," Evict 71 of Process A to 0,")
                        // fprintf(fw,"Change : %d\n", change_disk_page);
                        // fprintf(fw,"No free frame\n");
                    }
                    else if(frame_policy == 2) //LOCAL
                    {
                        // frame = i;
                        // fprintf(fw,"ji\n");
                        int change_frame = i;
                        for(int m = 0; m < frame_num; m++)
                        {
                            if(physical_memory_list[m][0] == now_process && memory_localfifo[m] < change_frame)
                            {
                                // fprintf(fw,"m:%d\n",m);
                                change_frame = memory_localfifo[m];
                                frame = m;
                            }
                        }
                        for(int m = 0; m < 32; m++)
                        {
                            if(tlb[m][1] == frame)
                            {
                                tlb[m][0] = -1;
                                tlb[m][1] = -1;
                                tlb[m][2] = -1;
                            }
                        }
                        for(int m = 0; m < process_num*page_num; m++)
                        {
                            if(disk[m][1] == -1)
                            {
                                change_disk_page = m;
                                disk[m][0] = physical_memory_list[frame][0];
                                disk[m][1] = physical_memory_list[frame][1];
                                break;
                            }
                        }
                        // fprintf(fw,"Change : %d\n", change_frame);
                        // frame = change_frame;
                        // memory_localfifo[frame] = i;
                        // memset(physical_memory_list[frame],0,10) ;
                        // fprintf(fw,"Change : %d\n", frame);
                    }
                }
                else if (page_policy == 1) //CLOCK
                {
                    if(frame_policy == 1) //GLOBAL
                    {
                        // printf("d\n");
                        int finded = 0;
                        int p = -1;
                        for(int j = now_kick; j < now_kick + frame_num; j++)
                        {
                            j %= frame_num;
                            // fprintf(fw,"%d\n",j);
                            int temp_process  = physical_memory_list[j][0];
                            int temp  = physical_memory_list[j][1];
                            // fprintf(fw,"%c\t%d\n",temp_process+65,temp);
                            if(page_table[temp_process][temp][2] == 1)
                            {
                                page_table[temp_process][temp][2] = 0;
                            }
                            else if(page_table[temp_process][temp][2] == 0)
                            {

                                // printf("change\n");
                                p = temp_process;
                                now_kick = j+1;
                                frame = j;
                                finded = 1;
                                break;
                            }
                        }
                        if(finded == 0)
                        {
                            for(int j = now_kick; j < now_kick + frame_num; j++)
                            {

                                j %= frame_num;
                                // fprintf(fw,"%d\n",j);
                                int temp_process  = physical_memory_list[j][0];
                                int temp  = physical_memory_list[j][1];
                                // fprintf(fw,"%c\t%d\n",temp_process+65,temp);
                                if(page_table[temp_process][temp][2] == 0)
                                {

                                    // printf("change\n");
                                    p = temp_process;
                                    now_kick = j+1;
                                    frame = j;
                                    break;
                                }
                            }
                        }
                        for(int m = 0; m < 32; m++)
                        {
                            if(tlb[m][1] == frame && p == now_process)
                            {
                                tlb[m][0] = -1;
                                tlb[m][1] = -1;
                                tlb[m][2] = -1;
                            }
                        }
                        // fprintf(fw,"Change : %d\n", frame);
                        for(int m = 0; m < process_num*page_num; m++)
                        {
                            if(disk[m][1] == -1)
                            {
                                change_disk_page = m;
                                disk[m][0] = physical_memory_list[frame][0];
                                disk[m][1] = physical_memory_list[frame][1];
                                break;
                            }
                        }
                    }
                    else if(frame_policy == 2) //LOCAL
                    {
                        int finded = 0;
                        int p = -1;
                        for(int j = local_kick[now_process]; j < local_kick[now_process] + frame_num; j++)
                        {
                            j %= frame_num;
                            // fprintf(fw,"%d\n",j);
                            int temp_process  = physical_memory_list[j][0];
                            int temp  = physical_memory_list[j][1];
                            // fprintf(fw,"%c\t%d\n",temp_process+65,temp);
                            if(page_table[temp_process][temp][2] == 1 && temp_process == now_process)
                            {
                                page_table[temp_process][temp][2] = 0;
                            }
                            else if(page_table[temp_process][temp][2] == 0 && temp_process == now_process)
                            {

                                // printf("change\n");
                                p = temp_process;
                                local_kick[now_process] = j+1;
                                frame = j;
                                finded = 1;
                                break;
                            }
                        }
                        if(finded == 0)
                        {
                            for(int j = local_kick[now_process]; j < local_kick[now_process] + frame_num; j++)
                            {
                                j %= frame_num;
                                // fprintf(fw,"%d\n",j);
                                int temp_process  = physical_memory_list[j][0];
                                int temp  = physical_memory_list[j][1];
                                // fprintf(fw,"%c\t%d\n",temp_process+65,temp);
                                if(page_table[temp_process][temp][2] == 0 && temp_process == now_process)
                                {

                                    // printf("change\n");
                                    p = temp_process;
                                    local_kick[now_process] = j+1;
                                    frame = j;
                                    break;
                                }
                            }
                        }
                        for(int m = 0; m < 32; m++)
                        {
                            if(tlb[m][1] == frame && p == now_process)
                            {
                                tlb[m][0] = -1;
                                tlb[m][1] = -1;
                                tlb[m][2] = -1;
                            }
                        }
                        // fprintf(fw,"Change : %d\n", frame);
                        for(int m = 0; m < process_num*page_num; m++)
                        {
                            if(disk[m][1] == -1)
                            {
                                change_disk_page = m;
                                disk[m][0] = physical_memory_list[frame][0];
                                disk[m][1] = physical_memory_list[frame][1];
                                break;
                            }
                        }
                    }
                }
            }
            if(frame != -1)
            {

                page_table[pro_int][address][2] = 1;
                fprintf(fw," %d,",frame) ;
                int vic_process = -1,vic_page = -1;
                for(int m = 0; m < process_num; m++)
                {
                    for(int n = 0; n < page_num; n++)
                    {
                        if(page_table[m][n][0] == frame)
                        {
                            page_table[m][n][0] = -1;
                            vic_process = m;
                            vic_page = n;
                        }
                    }
                }
                for(int j = 0; j < page_num*process_num; j++)
                {
                    if(disk[j][0] == now_process && disk[j][1] == address)
                    {
                        file_source = j;
                        disk[j][0] = -1;
                        disk[j][1] = -1;
                    }
                }
                if (vic_process == -1)
                {
                    vic_process = now_process;
                }
                char temp = physical_memory_list[frame][0] + 65;
                fprintf(fw," Evict %d of Process %c to %d, %d<<%d",vic_page,vic_process+65,change_disk_page,address,file_source);
                page_table[pro_int][address][0] = frame;
                page_table[pro_int][address][2] = 1;
                physical_memory_list[frame][0] = now_process;
                physical_memory_list[frame][1] = address;
                // fprintf(fw,"%d\t%d\n",physical_memory_list[frame][0] ,physical_memory_list[frame][1]);
                // memset(physical_memory_list[frame],0,10) ;
                // strcat(physical_memory_list[frame],physical);
                free_frame_list[frame] = 1;
                memory_localfifo[frame] = i;
                // fprintf(fw,"%s\n",physical_memory_list[frame]);
                physical_now += 1;
                // fprintf(fw,"tlb is hit %d\n",tlb_hit);
            }
        }
        if (tlb_hit == 0) //refill tlb
        {
            int tlb_full = 0;
            for(int j = 0 ; j < 32; j++)
            {
                if (tlb[j][1] == -1) //TLB is not full
                {
                    tlb_full = 1;
                    tlb[j][0] = address;
                    tlb[j][1] = page_table[pro_int][address][0];
                    tlb[j][2] = 0;
                    for(int n = 0; n < 32; n++)
                    {
                        if (n != j && tlb[n][1] != -1)
                        {
                            tlb[n][2] += 1;
                        }
                    }
                    fprintf(fw,"\nProcess %c, TLB Hit, %d=>%d\n",trace[i][10],tlb[j][0],tlb[j][1]);
                    break;
                }
            }
            if(tlb_full == 0)
            {
                //tlb滿了
                if (tlb_policy == 2) //random
                {
                    int t = rand()%32;
                    tlb[t][0] = address;
                    tlb[t][1] = page_table[pro_int][address][0];
                    tlb[t][2] = 0;
                    for(int n = 0; n < 32; n++)
                    {
                        if (n != t)
                        {
                            tlb[n][2] += 1;
                        }
                    }
                    fprintf(fw,"\nProcess %c, TLB Hit, %d=>%d\n",trace[i][10],tlb[t][0],tlb[t][1]);
                    tlb_hit = 1;

                }
                if(tlb_policy == 1) //LRU
                {
                    // ffprintf(fw,fw,"refill tlb\n");
                    // fprintf(fw,"tlb is full");
                    int t = 0;
                    int lru = tlb[0][2];
                    for(int m = 0; m < 32; m++) //find lru
                    {
                        if (lru < tlb[m][2])
                        {
                            lru = tlb[m][2];
                            t = m;
                        }
                    }
                    tlb[t][0] = address;
                    tlb[t][1] = page_table[pro_int][address][0];
                    tlb[t][2] = 0;
                    for(int n = 0; n < 32; n++)
                    {
                        if (n != t)
                        {
                            tlb[n][2] += 1;
                        }
                    }
                    count_fault[now_process][2] += 1;
                    fprintf(fw,"\nProcess %c, TLB Hit, %d=>%d\n",trace[i][10],tlb[t][0],tlb[t][1]);
                    tlb_hit = 1;						// fprintf(fw,"switch success\n");

                }
            }
        }
        last_process = now_process;//結尾紀錄process
        // for(int i = 0; i < 32; i++){
        // 	fprintf(fw,"\n disk process:%d address :%d lru:%d",tlb[i][0],tlb[i][1],tlb[i][2]);
        // }
    }
    fclose(fw);

    FILE *fe = fopen("./analysis.txt", "w+");
    for(int i = 0; i < process_num; i++)
    {
        float fr = count_fault[i][1]/count_fault[i][0];
        float hr = hit_rate[i][1]/hit_rate[i][0];
        // printf("%f\t%f\n",hit_rate[i][1],hit_rate[i][0]);
        float time = hr * 120 + (1-hr) * (220);
        fprintf(fe,"Process %c, Effective Access Time = %.3f\n",i+65,time);
        fprintf(fe,"Process %c, Page Fault Rate: %.3f\n",i+65,fr);
    }
    fclose(fe);
    return 0;

}
