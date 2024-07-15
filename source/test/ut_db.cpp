#include <gtest/gtest.h>
#include <spn/db.h>

TEST(db, add)
{
    struct db_ctx ctx;

    db_init(&ctx);
    for (int i = 0; i < DB_MAX_INTERFACE; i++) {
        struct db_interface* interface;
        ASSERT_EQ(db_add_interface(&ctx, i), SPN_OK);
        ASSERT_EQ(db_get_interface(&ctx, i, &interface), SPN_OK);
        ASSERT_NE(db_get_interface(&ctx, i + 1, &interface), SPN_OK);

        for (int j = 0; j < DB_MAX_PORT; j++) {
            struct db_port* port;
            ASSERT_EQ(db_add_port(interface, j), SPN_OK);
            ASSERT_EQ(db_get_port(interface, j, &port), SPN_OK);
            ASSERT_NE(db_get_port(interface, j + 1, &port), SPN_OK);
        }
    }
    ASSERT_EQ(db_add_interface(&ctx, DB_MAX_INTERFACE), -SPN_ENOMEM);
}

TEST(db, object)
{
    struct db_ctx ctx;
    db_value_t data;

    db_init(&ctx);

    /* u32 object */
    data.u32 = 0x12345678;
    db_add_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 1), 0, 0, 4, &data);

    /* u64 object */
    data.u64 = 0x1234567890abcdef;
    db_add_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 2), 0, 0, 8, &data);

    /* static str */
    strcpy(data.str, "abcdefg");
    db_add_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 3), 0, 1, 7, &data);

    /* dynamic str */
    data.ptr = malloc(32);
    strcpy((char*)data.ptr, "1234567890abcdef");
    db_add_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 4), 1, 1, 16, &data);

    /* assert */
    struct db_object* object;
    ASSERT_EQ(db_get_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 1), &object), SPN_OK);
    ASSERT_EQ(object->header.id, (db_id_t)(DB_ID_INVALID + 1));
    ASSERT_FALSE(db_is_array_object(object));
    ASSERT_TRUE(db_is_static_object(object));
    ASSERT_FALSE(db_is_static_string_object(object));
    ASSERT_EQ(object->data.u32, 0x12345678);

    ASSERT_EQ(db_get_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 2), &object), SPN_OK);
    ASSERT_EQ(object->header.id, (db_id_t)(DB_ID_INVALID + 2));
    ASSERT_FALSE(db_is_array_object(object));
    ASSERT_TRUE(db_is_static_object(object));
    ASSERT_FALSE(db_is_static_string_object(object));
    ASSERT_EQ(object->data.u64, 0x1234567890abcdef);

    ASSERT_EQ(db_get_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 3), &object), SPN_OK);
    ASSERT_EQ(object->header.id, (db_id_t)(DB_ID_INVALID + 3));
    ASSERT_TRUE(db_is_array_object(object));
    ASSERT_TRUE(db_is_static_object(object));
    ASSERT_TRUE(db_is_static_string_object(object));
    ASSERT_STREQ(object->data.str, "abcdefg");

    ASSERT_EQ(db_get_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 4), &object), SPN_OK);
    ASSERT_TRUE(db_is_array_object(object));
    ASSERT_FALSE(db_is_static_object(object));
    ASSERT_FALSE(db_is_static_string_object(object));
    ASSERT_STREQ((char*)object->data.ptr, "1234567890abcdef");

    ASSERT_EQ(db_get_global_object(&ctx, (db_id_t)(DB_ID_INVALID + 1), &object), SPN_OK);

    ASSERT_EQ(db_del_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 1)), SPN_OK);
    ASSERT_EQ(db_del_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 2)), SPN_OK);
    ASSERT_EQ(db_del_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 3)), SPN_OK);
    ASSERT_EQ(db_del_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 4)), SPN_OK);
    ASSERT_NE(db_del_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 4)), SPN_OK);
}

TEST(db, mem_leak_1)
{
    struct db_ctx ctx;
    db_value_t data;

    db_init(&ctx);

    /* Loop many times to check memory leak */
    for (int i = 0; i < 1e6; i++) {
        data.ptr = malloc(1024);
        ASSERT_NE(data.ptr, nullptr);
        db_add_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 1), 1, 1, 1024, &data);
        db_del_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 1));
    }
}

TEST(db, mem_leak_2)
{
    struct db_ctx ctx;
    db_value_t data;

    /* Loop many times to check memory leak */
    for (int i = 0; i < 1e6; i++) {
        data.ptr = malloc(1024);
        ASSERT_NE(data.ptr, nullptr);
        db_init(&ctx);
        db_add_object(&ctx.objects, (db_id_t)(DB_ID_INVALID + 1), 1, 1, 1024, &data);
        db_deinit(&ctx);
    }
}
TEST(db, get_interface_object)
{
    struct db_ctx ctx;
    db_value_t data;

    db_init(&ctx);

    /* Add interface */
    ASSERT_EQ(db_add_interface(&ctx, 0), SPN_OK);

    /* Add object to the interface */
    data.u32 = 0x12345678;
    ASSERT_EQ(db_add_object(&ctx.interfaces[0].objects, (db_id_t)(DB_ID_INVALID + 1), 0, 0, 4, &data), SPN_OK);

    /* Get the object from the interface */
    struct db_object* object;
    ASSERT_EQ(db_get_interface_object(&ctx, 0, (db_id_t)(DB_ID_INVALID + 1), &object), SPN_OK);
    ASSERT_EQ(object->header.id, (db_id_t)(DB_ID_INVALID + 1));
    ASSERT_FALSE(db_is_array_object(object));
    ASSERT_TRUE(db_is_static_object(object));
    ASSERT_FALSE(db_is_static_string_object(object));
    ASSERT_EQ(object->data.u32, 0x12345678);

    /* Clean up */
    db_deinit(&ctx);
}

TEST(db, get_port_object)
{
    struct db_ctx ctx;
    db_value_t data;

    db_init(&ctx);

    /* Add interface */
    ASSERT_EQ(db_add_interface(&ctx, 0), SPN_OK);

    /* Add port */
    ASSERT_EQ(db_add_port(&ctx.interfaces[0], 0), SPN_OK);

    /* Add object to the port */
    data.u32 = 0x12345678;
    ASSERT_EQ(db_add_object(&ctx.interfaces[0].ports[0].objects, (db_id_t)(DB_ID_INVALID + 1), 0, 0, 4, &data), SPN_OK);

    /* Get the object from the port */
    struct db_object* object;
    ASSERT_EQ(db_get_port_object(&ctx, 0, 0, (db_id_t)(DB_ID_INVALID + 1), &object), SPN_OK);
    ASSERT_EQ(object->header.id, (db_id_t)(DB_ID_INVALID + 1));
    ASSERT_FALSE(db_is_array_object(object));
    ASSERT_TRUE(db_is_static_object(object));
    ASSERT_FALSE(db_is_static_string_object(object));
    ASSERT_EQ(object->data.u32, 0x12345678);

    /* Clean up */
    db_deinit(&ctx);
}
