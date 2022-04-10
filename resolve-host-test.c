//
// Created by dran on 4/9/22.
//

#include "worker.h"


int test_blocked_sites() {
    struct resource_info resources;
    struct addrinfo* resolved_address;
    enum host_status resolved_status;

    CACHE_TTL = 10;

    resources = create_shared_resource(10, 10);

    block_table_add(resources.block_table, "very.bad.site.tv");
    block_table_add(resources.block_table, "23.31.83.227");  // foo.prinmath.com

    printf("block table initialized\n");

    resolved_status = resolve_host(&resources, "very.bad.site.tv", &resolved_address);
    printf("very.bad.site.tv status : %d/%d\n", resolved_status, blocked);

    resolved_status = resolve_host(&resources, "foo.prinmath.com", &resolved_address);
    printf("foo.prinmath.com status : %d/%d\n", resolved_status, blocked);

    resolved_status = resolve_host(&resources, "pornhub.com", &resolved_address);
    printf("pornhub.com status : %d/%d\n", resolved_status, ok);  // pornhub is ok but principia is blocked!? >_<

    return 0;
}


int main(int arc, char* argv[]) {
    return test_blocked_sites();
}