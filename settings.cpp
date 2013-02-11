/****************************************************************************
**
** Copyright 2010 Adminotech Ltd
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
** http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
*****************************************************************************/

#include "settings.h"
#include <stdio.h>
#include <QHash>

Settings::Settings()
{
}

Settings::~Settings()
{
}

bool Settings::readXmlFile(QStringList &name, QStringList &ipaddress, QStringList &port, QStringList &password)
{
    QDomDocument doc( "remotesettings" );
    QFile file( "remoteom_settings.xml" );
    if( !file.open(QIODevice::ReadOnly) )
        return false;
    if( !doc.setContent( &file ) )
    {
        file.close();
        return false;
    }

    file.close();

    QDomElement root = doc.documentElement();
    if( root.tagName() != "settings" )
        return false;

    QDomNode n = root.firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if( !e.isNull() )
        {
            if( e.tagName() == "address")
            {
                name << e.attribute("name", "");
                ipaddress << e.attribute( "ipaddress", "" );
                port << e.attribute( "port", "" );
                password << e.attribute( "password", "" );
            }
        }
        n = n.nextSibling();
    }

    return true;
}

bool Settings::readXmlFile(QHash<QString, QString> &vals)
{
    QDomDocument doc( "remotesettings" );
    QFile file( "remoteom_settings.xml" );
    if( !file.open(QIODevice::ReadOnly) )
        return false;
    if( !doc.setContent( &file ) )
    {
        file.close();
        return false;
    }

    file.close();

    QDomElement root = doc.documentElement();
    if( root.tagName() != "settings" )
        return false;

    QDomNode n = root.firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if( !e.isNull() )
        {
            if( e.tagName() == "address")
            {
                vals.insert(e.attribute("name", ""), e.attribute( "ipaddress", "" )+";"+e.attribute( "port", "" )+";"+e.attribute( "password", "" ));
            }
        }
        n = n.nextSibling();
    }

    return true;
}

bool Settings::readPassword(QString &password)
{
    QDomDocument doc( "remotesettings" );
    QFile file( "remoteom_settings.xml" );
    if( !file.open(QIODevice::ReadOnly) )
        return false;

    if( !doc.setContent( &file ) )
    {
        file.close();
        return false;
    }

    file.close();

    QDomElement root = doc.documentElement();
    if( root.tagName() != "settings" )
        return false;

    QDomNode n = root.firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if( !e.isNull() )
        {
            if( e.tagName() == "address" )
            {
                password = e.attribute( "password", "" );
            }
        }
        n = n.nextSibling();
    }

    return true;
}

bool Settings::deleteXml(QString &delete_name, QString &delete_IP)
{
    QDomDocument doc("remotesettings");
    QFile file("remoteom_settings.xml");
    if( !file.open(QIODevice::ReadWrite) )
        return false;

    if( !doc.setContent( &file ) )
    {
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomNodeList domList = root.elementsByTagName( "address" );
    if( root.tagName() != "settings" )
        return false;
    QDomNode n = root.firstChild();
    if ( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if( !e.isNull() )
        {
            if( e.tagName() == "address")
            {
                if (!domList.isEmpty())
                {
                    for (uint i = 0; i < domList.length(); i++ )
                    {
                        if ( domList.at(i).toElement().attribute("name") == delete_name &&
                             domList.at(i).toElement().attribute("ipaddress") == delete_IP)
                        {
                            root.removeChild(domList.at(i));
                            file.open(QIODevice::WriteOnly | QIODevice::Truncate );
                            QTextStream ts(&file);
                            root.save(ts,QDomNode::EncodingFromTextStream);
                            file.close();

                            if (domList.isEmpty())
                            {
                                file.open(QIODevice::WriteOnly | QIODevice::Truncate );
                                root.save(ts,QDomNode::EncodingFromTextStream);
                                file.close();
                                file.remove();
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool Settings::saveXmlFile(QHash<QString, QString> items)
{
    //Clear the file and save all

    QFile fileold("remoteom_settings.xml");
    fileold.remove();
    QFile file("remoteom_settings.xml");
    if(!file.open(QIODevice::ReadWrite))
    {
        qWarning("Open --- readwrite --err");
    }

    QDomDocument doc("remotesettings");
    QDomElement root = doc.createElement("settings");
    doc.appendChild(root);

    for (int i = 0; i < items.count(); i++ )
    {
        QStringList list = items.values().at(i).split(";");
        if (!list.isEmpty())
        {
            QDomElement cn = doc.createElement("address");
            cn.setAttribute("name", items.keys().at(i));
            cn.setAttribute("ipaddress", list[0]);
            cn.setAttribute("port", list[1]);
            cn.setAttribute("password", list[2]);

            root.appendChild(cn);
        }
    }

    QTextStream ts(&file);
    root.save(ts,QDomNode::EncodingFromTextStream);
    file.close();

    return true;
}
