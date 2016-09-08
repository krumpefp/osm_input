import json

def requestCondition():
    condition = dict()
    condition["tags"] = list()

    while True:
        tag = dict()
        key = input("Tag key (default = amenity) or 'q' to finish: ")
        if key == "":
            key = "amenity"
        if key == "q":
            break
        value = input("Tag value: ")
        tag["tag"] = key
        tag["value"] = value
        condition["tags"].append(tag)

    return condition


def requestIcon():
    name = input("Enter the icon name: ")
    while name == "":
        name = input("Enter the (nonempty) icon name: ")

    while True:
        szFac = input("Enter a size factor: ")
        try:
            fac = int(szFac)
            break
        except ValueError:
            print("Could not parse. Try again ...")

    icon = dict()
    icon["icon"] = name
    icon["factor"] = fac
    icon["conditions"] = list()

    while True:
        print("Condition input:")
        icon["conditions"].append(requestCondition())

        another = input("Add another condition (y, n)? ")
        if another == "n":
            break;

    return icon


if __name__ == "__main__":
    icons = list()

    while True:
        icon = requestIcon()

        print("Constructed the following icon: \n\t{}".format(json.dumps(icon)))

        proceed = input("Press 'c'ontinue, 's'kip or 'f'inish? ")
        if proceed == "c":
            print("Saving icon and continuing process")
            icons.append(icon)
        elif proceed == "f":
            print("Saving icon and finishing process")
            icons.append(icon)
            break;
        else:
            print("Dropping icon!")

    print("Collected these icons: {}".format(json.dumps(icons)))

    filePath = input("Give a file path if you want to save your result: ")
    if filePath != "":
        with open(filePath, "w") as outfile:
            json.dump(icons, outfile, sort_keys = True, indent = 4, separators=(',', ': '))
