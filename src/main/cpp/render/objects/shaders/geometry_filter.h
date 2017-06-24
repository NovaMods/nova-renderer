#ifndef GEOMETRY_FILTER_H
#define GEOMETRY_FILTER_H

#include <vector>
#include <string>
#include <optional.hpp>
#include <unordered_map>
#include "../render_object.h"

namespace nova {
    class igeometry_filter {
    public:
        /*!
         * \brief Checks if this filter matches the block or not
         *
         * \param block The block to check for matching
         * \return True if the block matches, false if it does not
         */
        virtual bool matches(const mc_block_definition& block) const = 0;

        virtual bool matches(const render_object& obj) const = 0;

        virtual std::string to_string() const = 0;
    };

    class and_geometry_filter : public igeometry_filter {
    public:
        and_geometry_filter(std::shared_ptr<igeometry_filter> filter1, std::shared_ptr<igeometry_filter> filter2);

        /*!
         * \brief Checks if the given block matches this filter
         * \param block The block to check for matching
         * \return True if the block matches both filters, false otherwise
         */
        bool matches(const mc_block_definition& block) const;

        bool matches(const render_object& obj) const;

        std::string to_string() const;

    private:
        std::shared_ptr<igeometry_filter> filter1;
        std::shared_ptr<igeometry_filter> filter2;
    };

    class or_geometry_filter : public igeometry_filter {
    public:
        or_geometry_filter(std::shared_ptr<igeometry_filter> filter1, std::shared_ptr<igeometry_filter> filter2);

        /*!
         * \brief Checks if the given block matches this filter
         * \param block The block to check for matching
         * \return True if the block matches either filters, false otherwise
         */
        bool matches(const mc_block_definition& block) const;

        bool matches(const render_object& obj) const;

        std::string to_string() const;

    private:
        std::shared_ptr<igeometry_filter> filter1;
        std::shared_ptr<igeometry_filter> filter2;
    };

    class name_geometry_filter : public igeometry_filter {
    public:
        name_geometry_filter(std::string name);

        bool matches(const mc_block_definition& block) const;

        bool matches(const render_object& obj) const;

        std::string to_string() const;
    private:
        std::string name;
    };

    class name_part_geometry_filter : public igeometry_filter {
    public:
        name_part_geometry_filter(std::string name_part);

        bool matches(const mc_block_definition& block) const;

        bool matches(const render_object& obj) const;

        std::string to_string() const;
    private:
        std::string name_part;
    };

    class geometry_type_geometry_filter : public igeometry_filter {
    public:
        geometry_type_geometry_filter(geometry_type type);

        bool matches(const mc_block_definition& block) const;

        bool matches(const render_object& obj) const;

        std::string to_string() const;
    private:
        geometry_type type;
    };

    class transparent_geometry_filter : public igeometry_filter {
    public:
        transparent_geometry_filter();
        transparent_geometry_filter(bool should_be_transparent);

        bool matches(const mc_block_definition& block) const;

        bool matches(const render_object& obj) const;

        std::string to_string() const;
    private:
        bool should_be_transparent;
    };

    class emissive_geometry_filter : public igeometry_filter {
    public:
        emissive_geometry_filter(bool should_be_emissive = true);

        bool matches(const mc_block_definition& block) const;

        bool matches(const render_object& obj) const;

        std::string to_string() const;
    private:
        bool should_be_emissive;
    };
}

#endif
